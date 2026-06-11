#pragma once
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <mbedtls/aes.h>
#include <string.h>
#include "config.h"

// ============================================================
// 协议常量
// ============================================================
#define CMD_GET_SESSION_ID  0xF0
#define CMD_GET_SECRET      0xF1
#define CMD_GET_AUTH        0xF2
#define CMD_OPEN_LOCK       0x01
#define CMD_GET_DNA_INFO    0x0C
#define CMD_SET_SYSTEM_TIME 0x07

#define FLAG_INITIAL    1
#define FLAG_WITH_KEY   2
#define FLAG_DYNAMIC    3

// ============================================================
// 帧结构
// ============================================================
struct BleFrame {
    uint32_t session_id  = 0;
    uint8_t  snr         = 0;
    uint8_t  rfu         = 0;
    uint8_t  cmd         = 0;
    uint8_t  status      = 0;
    uint8_t  flag        = 0;
    uint16_t key_group   = 900;
    uint16_t cmd_ver     = 12;
    uint16_t flag_proto  = FLAG_INITIAL;
    uint8_t  iterable[64];
    size_t   iter_len    = 0;
};

// ============================================================
// 门锁设备信息
// ============================================================
struct LockDevice {
    String name;
    String mac;          // BLE MAC "AA:BB:CC:DD:EE:FF"
    String lock_id_key;
    String aes_key;      // 16 字节密钥
    String auth_code;    // hex 认证码
    uint32_t begin_time; // API 返回的 beginTime (authStartTime)
    int    key_group;
};

// ============================================================
// BLE 通知回调 (静态)
// ============================================================
static uint8_t  _rx_buf[256];
static size_t   _rx_len = 0;
static bool     _got_response = false;

static void _notify_cb(BLERemoteCharacteristic* ch,
                       uint8_t* data, size_t len, bool is_notify) {
    if (_rx_len + len < sizeof(_rx_buf)) {
        memcpy(_rx_buf + _rx_len, data, len);
        _rx_len += len;
    }
    // 检查帧完整性: 3 字节头 + 2 字节总长
    if (_rx_len >= 5 && _rx_buf[0] == 'H' && _rx_buf[1] == 'S' && _rx_buf[2] == 'J') {
        uint16_t expected = (_rx_buf[3] << 8) | _rx_buf[4];
        if (_rx_len >= expected) {
            _got_response = true;
        }
    }
}

// ============================================================
// CRC16 (多项式 0xA001)
// ============================================================
static uint16_t _crc16(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            uint8_t t = crc & 0xFF;
            crc >>= 1;
            if (t & 1) crc ^= 0xA001;
        }
    }
    return crc;
}

// ============================================================
// AES-128-ECB + PKCS7
// ============================================================
static size_t _aes_encrypt(const uint8_t* plain, size_t plain_len,
                           const uint8_t* key16, uint8_t* out) {
    size_t pad = 16 - (plain_len % 16);
    size_t total = plain_len + pad;
    uint8_t* padded = (uint8_t*)malloc(total);
    memcpy(padded, plain, plain_len);
    memset(padded + plain_len, pad, pad);

    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_enc(&ctx, key16, 128);

    for (size_t i = 0; i < total; i += 16) {
        mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, padded + i, out + i);
    }
    mbedtls_aes_free(&ctx);
    free(padded);
    return total;
}

static size_t _aes_decrypt(const uint8_t* cipher, size_t cipher_len,
                           const uint8_t* key16, uint8_t* out) {
    mbedtls_aes_context ctx;
    mbedtls_aes_init(&ctx);
    mbedtls_aes_setkey_dec(&ctx, key16, 128);

    for (size_t i = 0; i < cipher_len; i += 16) {
        mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_DECRYPT, cipher + i, out + i);
    }
    mbedtls_aes_free(&ctx);

    uint8_t pad = out[cipher_len - 1];
    if (pad >= 1 && pad <= 16) return cipher_len - pad;
    return cipher_len;
}

// ============================================================
// 帧编码
// ============================================================
static uint8_t _snr_counter = 0;

static size_t _encode_frame(const BleFrame& f, const uint8_t* aes_key,
                            uint8_t* out_buf) {
    // 明文: session_id(4) + snr(1) + rfu(1) + cmd(1) + status(1)
    //       + flag(1) + key_group(2) + cmd_ver(2) + iterable
    uint8_t plain[128];
    size_t p = 0;

    plain[p++] = (f.session_id >> 24) & 0xFF;
    plain[p++] = (f.session_id >> 16) & 0xFF;
    plain[p++] = (f.session_id >> 8)  & 0xFF;
    plain[p++] = f.session_id & 0xFF;
    plain[p++] = _snr_counter++;
    plain[p++] = f.rfu;
    plain[p++] = f.cmd;
    plain[p++] = f.status;
    plain[p++] = f.flag;
    plain[p++] = (f.key_group >> 8) & 0xFF;
    plain[p++] = f.key_group & 0xFF;
    plain[p++] = (f.cmd_ver >> 8) & 0xFF;
    plain[p++] = f.cmd_ver & 0xFF;
    if (f.iter_len > 0) {
        memcpy(plain + p, f.iterable, f.iter_len);
        p += f.iter_len;
    }

    // AES 加密
    uint8_t encrypted[128];
    size_t enc_len = _aes_encrypt(plain, p, aes_key, encrypted);

    // 组帧: HSJ + total_len(2) + flag_proto(2) + encrypted + crc(2)
    size_t o = 0;
    out_buf[o++] = 'H';
    out_buf[o++] = 'S';
    out_buf[o++] = 'J';
    uint16_t total = 9 + enc_len;
    out_buf[o++] = (total >> 8) & 0xFF;
    out_buf[o++] = total & 0xFF;
    out_buf[o++] = (f.flag_proto >> 8) & 0xFF;
    out_buf[o++] = f.flag_proto & 0xFF;
    memcpy(out_buf + o, encrypted, enc_len);
    o += enc_len;

    uint16_t crc = _crc16(out_buf, o);
    out_buf[o++] = (crc >> 8) & 0xFF;
    out_buf[o++] = crc & 0xFF;
    return o;
}

// ============================================================
// 帧解码
// ============================================================
static bool _decode_frame(const uint8_t* data, size_t len,
                          const uint8_t* aes_key, BleFrame& out) {
    if (len < 9) return false;
    const uint8_t* encrypted = data + 7;
    size_t enc_len = len - 9;

    uint8_t plain[128];
    size_t plain_len = _aes_decrypt(encrypted, enc_len, aes_key, plain);
    if (plain_len < 13) return false;

    size_t p = 0;
    out.session_id = ((uint32_t)plain[p]   << 24) |
                     ((uint32_t)plain[p+1] << 16) |
                     ((uint32_t)plain[p+2] << 8)  | plain[p+3]; p += 4;
    out.snr       = plain[p++];
    out.rfu       = plain[p++];
    out.cmd       = plain[p++];
    out.status    = plain[p++];
    out.flag      = plain[p++];
    out.key_group = (plain[p] << 8) | plain[p+1]; p += 2;
    out.cmd_ver   = (plain[p] << 8) | plain[p+1]; p += 2;
    size_t remaining = plain_len - p;
    out.iter_len  = (remaining < 64) ? remaining : 64;
    if (out.iter_len > 0) memcpy(out.iterable, plain + p, out.iter_len);
    return true;
}

// ============================================================
// hex string → bytes
// ============================================================
static size_t _hex_to_bytes(const char* hex, uint8_t* out) {
    size_t len = strlen(hex);
    size_t n = 0;
    for (size_t i = 0; i + 1 < len; i += 2) {
        char buf[3] = {hex[i], hex[i+1], 0};
        out[n++] = strtoul(buf, NULL, 16);
    }
    return n;
}

// ============================================================
// LockService — 门锁控制
// ============================================================
class LockService {
public:
    LockDevice devices[8];
    int device_count = 0;
    int active_idx = 0;
    bool locked = true;
    String last_result = "";
    String user_token = "";

    // 显示回调 — 每次状态更新时调用, 用于实时刷新
    void (*on_status)(const char*) = nullptr;
    // 空闲回调 — 长操作中调用, 让 WebServer 处理请求
    void (*on_idle)() = nullptr;

    void set_token(const String& token) { user_token = token; }

    void set_status(const String& s) {
        last_result = s;
        Serial.println(s);
        if (on_status) on_status(s.c_str());
        if (on_idle) on_idle();
    }

    // ========================================================
    // 从 API 拉取门锁列表 + BLE 凭据
    // ========================================================
    bool fetch_credentials() {
        if (strlen(LOCK_TOKEN) < 5 || strlen(API_HOST) < 5) {
            set_status("ERR: config incomplete");
            return false;
        }

        // --- 1) 获取门锁列表 ---
        String resp = _call_api(API_METHOD_LIST, "{}");
        if (resp.isEmpty()) {
            set_status("ERR: API call failed (empty)");
            return false;
        }

        // 调试: 打印前 200 字符看返回了什么
        Serial.print("[API] raw response: ");
        Serial.println(resp.substring(0, 200));

        DynamicJsonDocument doc(4096);
        DeserializationError err = deserializeJson(doc, resp);
        if (err) {
            set_status("ERR: list JSON: " + String(err.c_str()));
            Serial.printf("[API] deserialize error: %s\n", err.c_str());
            return false;
        }

        int code = doc["resultCode"] | -1;
        if (code != 0) {
            set_status("ERR: API code=" + String(code));
            return false;
        }

        JsonArray list = doc["data"]["data"]["lockList"];
        if (list.isNull()) list = doc["data"]["lockList"];
        if (list.isNull()) {
            set_status("ERR: no lockList in response");
            return false;
        }

        device_count = 0;
        for (JsonObject item : list) {
            if (device_count >= 8) break;
            devices[device_count].lock_id_key = item["lockIdKey"].as<String>();
            devices[device_count].name        = item["lockName"].as<String>();
            devices[device_count].mac         = item["lockMac"].as<String>();
            device_count++;
        }
        Serial.printf("[API] locks: %d\n", device_count);

        if (device_count == 0) {
            set_status("ERR: no locks found");
            return false;
        }

        // --- 2) 获取 BLE 凭据 (aesKey + authCode) ---
        String key_data = "{\"lockIdKey\":[";
        for (int i = 0; i < device_count; i++) {
            if (i > 0) key_data += ",";
            key_data += "\"" + devices[i].lock_id_key + "\"";
        }
        key_data += "]}";

        resp = _call_api(API_METHOD_AUTH, key_data.c_str());
        if (resp.isEmpty()) {
            set_status("ERR: auth API call failed");
            return false;
        }

        DynamicJsonDocument doc2(4096);
        DeserializationError err2 = deserializeJson(doc2, resp);
        if (err2) {
            set_status("ERR: auth JSON: " + String(err2.c_str()));
            Serial.printf("[API] auth deserialize error: %s\n", err2.c_str());
            return false;
        }
        Serial.printf("[API] auth response: %s\n", resp.substring(0, 300).c_str());

        int code2 = doc2["resultCode"] | -1;
        if (code2 != 0) {
            set_status("ERR: auth API code=" + String(code2));
            return false;
        }

        JsonArray lists = doc2["data"]["data"]["lists"];
        if (lists.isNull()) lists = doc2["data"]["lists"];

        for (JsonObject item : lists) {
            String id = item["lockIdKey"].as<String>();
            for (int i = 0; i < device_count; i++) {
                if (devices[i].lock_id_key == id) {
                    devices[i].aes_key     = item["aesKey"].as<String>();
                    devices[i].auth_code   = item["authCode"].as<String>();
                    devices[i].begin_time  = item["beginTime"] | 0;
                    Serial.printf("[API] lock=%s key=%s auth=%s begin=%u\n",
                        devices[i].name.c_str(),
                        devices[i].aes_key.c_str(),
                        devices[i].auth_code.substring(0, 8).c_str(),
                        devices[i].begin_time);
                    break;
                }
            }
        }

        set_status("OK: " + String(device_count) + " lock(s) ready");
        return true;
    }

    // ========================================================
    // BLE 开锁流程 (六步握手)
    // ========================================================
    bool run_unlock_protocol(BLEClient* client,
                             BLERemoteCharacteristic* write_ch,
                             BLERemoteCharacteristic* notify_ch,
                             const LockDevice& dev) {
        // 初始密钥: 优先用 API 返回的 aesKey, 回退到默认
        uint8_t aes_key[16];
        if (dev.aes_key.length() == 16) {
            memcpy(aes_key, dev.aes_key.c_str(), 16);
            Serial.printf("[BLE] using API key: %s\n", dev.aes_key.c_str());
        } else {
            memcpy(aes_key, BLE_DEFAULT_KEY, 16);
            Serial.println("[BLE] using default key");
        }
        uint16_t flag_proto = FLAG_WITH_KEY;
        uint16_t cmd_ver = 12;
        Serial.printf("[BLE] cmd_ver=%d\n", cmd_ver);

        // Auth code bytes
        uint8_t auth_bytes[32];
        size_t auth_len = _hex_to_bytes(dev.auth_code.c_str(), auth_bytes);

        uint32_t session_id = 0;
        BleFrame resp;

        // ---- 1) GET_SESSION_ID ----
        set_status("handshake 1/6...");
        if (!_send_cmd(write_ch, notify_ch, CMD_GET_SESSION_ID, 0, 0,
                       flag_proto, aes_key, nullptr, 0, resp,
                       0, cmd_ver)) {
            set_status("FAIL: session id");
            return false;
        }
        session_id = resp.session_id;
        Serial.printf("[BLE] session_id=%u\n", session_id);

        // ---- 2) GET_SECRET (动态密钥) ----
        set_status("handshake 2/6...");
        if (!_send_cmd(write_ch, notify_ch, CMD_GET_SECRET, 0, 0,
                       flag_proto, aes_key, nullptr, 0, resp,
                       session_id, cmd_ver)) {
            set_status("FAIL: secret");
            return false;
        }
        memcpy(aes_key, resp.iterable, 16);
        flag_proto = FLAG_DYNAMIC;
        Serial.println("[BLE] dynamic key acquired");

        // ---- 3) GET_AUTH (认证) ----
        set_status("handshake 3/6...");
        if (!_send_cmd(write_ch, notify_ch, CMD_GET_AUTH, 0, 0,
                       flag_proto, aes_key, auth_bytes, auth_len, resp,
                       session_id, cmd_ver)) {
            set_status("FAIL: auth");
            return false;
        }
        Serial.println("[BLE] authenticated");

        // ---- 4) GET_DNA_INFO (获取锁的协议版本) ----
        set_status("handshake 4/6...");
        if (!_send_cmd(write_ch, notify_ch, CMD_GET_DNA_INFO, 0, 0,
                       flag_proto, aes_key, nullptr, 0, resp,
                       session_id, cmd_ver)) {
            Serial.println("[BLE] DNA info not available");
        } else {
            if (resp.iter_len > 6) {
                uint8_t ble_ver = resp.iterable[6];
                Serial.printf("[BLE] DNA: bleProtocolVersion=%d\n", ble_ver);
            }
        }

        // ---- 5) SET_LOCK_SYSTEM_TIME (同步锁的时间) ----
        set_status("handshake 5/6...");
        {
            uint32_t now = (uint32_t)time(nullptr);
            int32_t tz_sec = 8 * 3600;
            uint8_t time_data[7];
            time_data[0] = (now >> 24) & 0xFF;
            time_data[1] = (now >> 16) & 0xFF;
            time_data[2] = (now >> 8)  & 0xFF;
            time_data[3] = now & 0xFF;
            time_data[4] = (tz_sec >> 16) & 0xFF;
            time_data[5] = (tz_sec >> 8)  & 0xFF;
            time_data[6] = tz_sec & 0xFF;
            _send_cmd(write_ch, notify_ch, CMD_SET_SYSTEM_TIME, 0, 0,
                      flag_proto, aes_key, time_data, 7, resp,
                      session_id, cmd_ver);
        }

        // ---- 6) OPEN_LOCK ----
        set_status("handshake 6/6...");
        uint32_t epoch = (uint32_t)time(nullptr);
        int32_t tz_seconds = 8 * 3600;
        uint32_t auth_start = dev.begin_time;
        uint8_t lock_data[15];
        memset(lock_data, 0, 4);
        lock_data[4] = (epoch >> 24) & 0xFF;
        lock_data[5] = (epoch >> 16) & 0xFF;
        lock_data[6] = (epoch >> 8)  & 0xFF;
        lock_data[7] = epoch & 0xFF;
        lock_data[8] = (tz_seconds >> 16) & 0xFF;
        lock_data[9] = (tz_seconds >> 8)  & 0xFF;
        lock_data[10] = tz_seconds & 0xFF;
        lock_data[11] = (auth_start >> 24) & 0xFF;
        lock_data[12] = (auth_start >> 16) & 0xFF;
        lock_data[13] = (auth_start >> 8)  & 0xFF;
        lock_data[14] = auth_start & 0xFF;

        if (!_send_cmd(write_ch, notify_ch, CMD_OPEN_LOCK, 6, 8,
                       flag_proto, aes_key, lock_data, 15, resp,
                       session_id, 21)) {
            set_status("FAIL: unlock cmd");
            return false;
        }

        Serial.printf("[BLE] unlock status=%d\n", resp.status);
        if (resp.status == 1) return true;

        // v21 失败降级到 v12
        Serial.println("[BLE] v21 rejected, trying v12...");
        set_status("retry v12...");
        uint8_t lock_data_v12[11];
        memset(lock_data_v12, 0, 4);
        lock_data_v12[4] = (epoch >> 24) & 0xFF;
        lock_data_v12[5] = (epoch >> 16) & 0xFF;
        lock_data_v12[6] = (epoch >> 8)  & 0xFF;
        lock_data_v12[7] = epoch & 0xFF;
        lock_data_v12[8] = (tz_seconds >> 16) & 0xFF;
        lock_data_v12[9] = (tz_seconds >> 8)  & 0xFF;
        lock_data_v12[10] = tz_seconds & 0xFF;

        if (!_send_cmd(write_ch, notify_ch, CMD_OPEN_LOCK, 6, 8,
                       flag_proto, aes_key, lock_data_v12, 11, resp,
                       session_id, 12)) {
            set_status("FAIL: unlock cmd v12");
            return false;
        }

        Serial.printf("[BLE] unlock v12 status=%d\n", resp.status);
        if (resp.status == 1) return true;

        set_status("FAIL: lock rejected (status=" + String(resp.status) + ")");
        return false;
    }

    // ========================================================
    // 开锁入口
    // ========================================================
    bool open_lock(int idx = -1) {
        if (idx < 0) idx = active_idx;
        if (idx < 0 || idx >= device_count) {
            set_status("ERR: no device");
            return false;
        }

        LockDevice& dev = devices[idx];
        set_status("scanning " + dev.mac + "...");

        // 1) BLE 扫描
        BLEAddress target_addr;
        bool found = _scan_device(dev.mac, target_addr);
        if (!found) {
            set_status("ERR: not found " + dev.mac);
            return false;
        }

        // 2) 连接
        BLEClient* c = BLEDevice::createClient();
        if (!c->connect(target_addr)) {
            set_status("ERR: connect failed");
            return false;
        }

        // 3) 发现服务
        BLERemoteService* svc = c->getService(BLEUUID(SERVICE_UUID));
        if (!svc) {
            set_status("ERR: service not found");
            c->disconnect();
            return false;
        }
        BLERemoteCharacteristic* w = svc->getCharacteristic(BLEUUID(WRITE_UUID));
        BLERemoteCharacteristic* n = svc->getCharacteristic(BLEUUID(NOTIFY_UUID));
        if (!w || !n) {
            set_status("ERR: characteristic not found");
            c->disconnect();
            return false;
        }

        // 4) 注册通知
        n->registerForNotify(_notify_cb);
        delay(200);

        // 5) 执行开锁协议
        bool ok = run_unlock_protocol(c, w, n, dev);

        if (ok) {
            set_status("OK: unlocked");
            locked = false;
        } else {
            if (last_result.startsWith("FAIL:") || last_result.startsWith("ERR:")) {
                // keep specific error from protocol
            } else {
                set_status("FAIL: unlock failed");
            }
        }

        c->disconnect();
        return ok;
    }

    void init_ble() {
        if (_ble_inited) return;
        BLEDevice::init("ESP32-Lock");
        delay(500);
        _ble_inited = true;
    }

    void deinit_ble() {
        if (!_ble_inited) return;
        BLEDevice::deinit(true);
        _ble_inited = false;
        delay(300);
    }

private:
    // ========================================================
    // HTTPS API 调用
    // ========================================================
    String _call_api(const char* method, const char* data_json) {
        WiFiClientSecure client;
        client.setInsecure();

        if (!client.connect(API_HOST, API_PORT)) {
            Serial.println("[API] connect failed");
            return "";
        }

        String body = "{\"method\":\"";
        body += method;
        body += "\",\"tokenId\":\"";
        body += user_token.length() > 0 ? user_token : LOCK_TOKEN;
        body += "\",\"data\":";
        body += data_json;
        body += ",\"appCom\":\"";
        body += API_APP_ID;
        body += "\"}";

        client.println("POST / HTTP/1.1");
        client.print("Host: ");
        client.print(API_HOST);
        client.print(":");
        client.println(API_PORT);
        client.println("Content-Type: text/json;charset=utf-8");
        client.println("Content-Version: 6.5");
        client.print("Content-Length: ");
        client.println(body.length());
        client.println("Connection: close");
        client.println();
        client.print(body);

        String response = "";
        unsigned long t0 = millis();
        while (client.connected() && millis() - t0 < 10000) {
            if (client.available()) {
                response += client.readString();
            }
            delay(10);
            if (on_idle) on_idle();
        }
        client.stop();

        // 直接找第一个 { 来定位 JSON 正文
        int brace = response.indexOf('{');
        if (brace >= 0) {
            response = response.substring(brace);
        }
        return response;
    }

    // ========================================================
    // BLE 扫描 (返回 true 表示找到, 填充 addr)
    // ========================================================
    bool _scan_device(const String& mac, BLEAddress& out_addr) {
        String target = mac;
        target.toLowerCase();
        if (target.indexOf(':') < 0) {
            target = "";
            for (int i = 0; i < 12; i += 2) {
                if (i > 0) target += ":";
                target += mac.substring(i, i + 2);
            }
            target.toLowerCase();
        }

        // 最多扫 3 轮, 每轮 8 秒
        bool found = false;
        for (int attempt = 0; attempt < 3 && !found; attempt++) {
            set_status("scanning " + target + " (" + String(attempt + 1) + "/3)...");

            BLEScan* scan = BLEDevice::getScan();
            scan->setActiveScan(true);
            scan->start(8, false);

            int n = scan->getResults()->getCount();
            for (int i = 0; i < n; i++) {
                BLEAdvertisedDevice d = scan->getResults()->getDevice(i);
                String addr = String(d.getAddress().toString().c_str());
                addr.toLowerCase();
                if (addr == target) {
                    out_addr = d.getAddress();
                    found = true;
                    break;
                }
            }
            scan->stop();
            scan->clearResults();

            if (!found && attempt < 2) {
                // 重试间隔: 让 WebServer 处理请求
                for (int w = 0; w < 10; w++) {
                    delay(100);
                    if (on_idle) on_idle();
                }
            }
        }

        if (!found) Serial.println("[SCAN] target not found after 3 attempts");
        return found;
    }

    // ========================================================
    // 发送 BLE 命令并等待响应
    // ========================================================
    bool _send_cmd(BLERemoteCharacteristic* write_ch,
                   BLERemoteCharacteristic* notify_ch,
                   uint8_t cmd, uint8_t status, uint8_t flag,
                   uint16_t flag_proto, const uint8_t* aes_key,
                   const uint8_t* iter, size_t iter_len,
                   BleFrame& out_resp,
                   uint32_t session_id = 0,
                   uint16_t cmd_ver = 12) {
        BleFrame f;
        f.session_id = session_id;
        f.cmd_ver = cmd_ver;
        f.cmd = cmd;
        f.status = status;
        f.flag = flag;
        f.key_group = devices[active_idx].key_group > 0
                      ? devices[active_idx].key_group : 900;
        f.flag_proto = flag_proto;
        if (iter && iter_len > 0) {
            memcpy(f.iterable, iter, iter_len);
            f.iter_len = iter_len;
        }

        uint8_t frame[256];
        size_t frame_len = _encode_frame(f, aes_key, frame);

        _rx_len = 0;
        _got_response = false;

        // BLE 分片写入 (20 字节)
        for (size_t i = 0; i < frame_len; i += 20) {
            size_t chunk = (frame_len - i < 20) ? (frame_len - i) : 20;
            write_ch->writeValue(frame + i, chunk, false);
            delay(30);
            if (on_idle) on_idle();
        }

        // 等待响应 (最多 10 秒)
        unsigned long t0 = millis();
        while (!_got_response && millis() - t0 < 10000) {
            delay(20);
            if (on_idle) on_idle();
        }
        if (!_got_response) return false;

        return _decode_frame(_rx_buf, _rx_len, aes_key, out_resp);
    }

    static constexpr const char* SERVICE_UUID = "0000FFF0-0000-1000-8000-00805F9B34FB";
    static constexpr const char* WRITE_UUID   = "0000FFF1-0000-1000-8000-00805F9B34FB";
    static constexpr const char* NOTIFY_UUID  = "0000FFF2-0000-1000-8000-00805F9B34FB";

    bool _ble_inited = false;
};
