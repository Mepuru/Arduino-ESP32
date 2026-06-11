/*
 * BLE Door Lock - ESP32-S3 + ST7789
 *
 * 用手机浏览器控制 BLE 门锁开锁, 无需物理按钮.
 *
 * 接线:
 *   ST7789  →  ESP32-S3
 *   VCC     →  3.3V
 *   GND     →  GND
 *   CS      →  GPIO10
 *   DC      →  GPIO5
 *   RST     →  GPIO6
 *   SCL     →  GPIO12
 *   SDA     →  GPIO11
 *   BL      →  GPIO21
 *
 * 使用方法:
 *   1. 填好 config.h 中 WiFi / token 信息
 *   2. 编译上传, 查看 TFT 上的 IP 地址
 *   3. 手机浏览器访问该 IP → 点开锁
 */

#include <WiFi.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <time.h>
#include <Preferences.h>
#include "config.h"
#include "lock_service.h"
#include "web_ui.h"

// ============ 全局 ============
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);  // 帧缓冲, 防闪烁
WebServer server(80);
LockService lock;

static bool unlocked = false;
static unsigned long last_tft_update = 0;
static unsigned long unlock_time = 0;
static String wifi_ip = "";

// BLE 进度实时推送到屏幕 (update_display 在后面定义, 加前向声明)
static void update_display(bool force = false);

static void status_callback(const char* msg) {
    (void)msg;
    update_display(true);
}

// ============ NTP ============
void sync_time() {
    configTime(8 * 3600, 0, "ntp.aliyun.com", "pool.ntp.org");
    struct tm t;
    if (getLocalTime(&t, 3000)) {
        Serial.printf("[NTP] %04d-%02d-%02d %02d:%02d:%02d\n",
            t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
            t.tm_hour, t.tm_min, t.tm_sec);
    }
}

// ============ TFT 显示 ============
void update_display(bool force) {
    if (!force && millis() - last_tft_update < 2000) return;
    last_tft_update = millis();

    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);

    // 标题
    spr.setTextColor(TFT_CYAN, TFT_BLACK);
    spr.drawString("BLE Lock", 8, 4, 4);

    // IP
    spr.setTextColor(TFT_GREEN, TFT_BLACK);
    spr.drawString(wifi_ip, 8, 36, 2);

    // WiFi 信号
    int rssi = WiFi.RSSI();
    spr.setTextColor(rssi > -70 ? TFT_GREEN : TFT_YELLOW, TFT_BLACK);
    spr.drawString("WiFi: " + String(rssi) + " dBm", 8, 58, 2);

    // 门锁状态
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    if (lock.device_count > 0) {
        LockDevice& dev = lock.devices[lock.active_idx];
        spr.drawString("Lock: " + dev.name, 8, 84, 2);
    }

    // 进度/状态 (大字显示, 最多 3 行)
    int y = 115;
    String msg = lock.last_result;
    if (msg.length() > 0) {
        bool ok = msg.startsWith("OK");
        bool fail = msg.startsWith("FAIL") || msg.startsWith("ERR");

        // 大字号显示状态
        spr.setTextColor(ok ? TFT_GREEN : fail ? TFT_RED : TFT_YELLOW, TFT_BLACK);
        spr.setTextDatum(ML_DATUM);  // 左对齐垂直居中
        
        // 按长度分字体大小: 长文本用 font2, 短文本用 font4
        int font = (msg.length() > 20) ? 2 : 4;
        if (font == 4 && spr.textWidth(msg, 4) > 224) font = 2;
        spr.drawString(msg, 8, y + 12, font);
        
        spr.setTextDatum(TL_DATUM);
        y += (font == 4) ? 30 : 20;
    }

    // 第二步提示 (如果不在进行中, 显示操作指引)
    if (!msg.startsWith("handshake") && !msg.startsWith("scanning") 
        && !msg.startsWith("retry") && !msg.startsWith("FAIL") && !msg.startsWith("ERR")) {
        spr.setTextColor(TFT_YELLOW, TFT_BLACK);
        spr.drawString("Open browser:", 8, y, 2);
        y += 18;
        spr.setTextColor(TFT_CYAN, TFT_BLACK);
        spr.drawString("http://" + wifi_ip, 8, y, 2);
    }

    // 底部状态条
    bool busy = msg.startsWith("handshake") || msg.startsWith("scanning") || msg.startsWith("retry");
    spr.fillRect(0, 224, 240, 16, busy ? TFT_MAROON : TFT_NAVY);
    spr.setTextColor(TFT_WHITE, spr.color16to8(busy ? TFT_MAROON : TFT_NAVY));
    spr.drawString(busy ? "BUSY" : (unlocked ? "UNLOCKED" : "LOCKED"), 8, 226, 2);
    if (lock.device_count > 0) {
        char buf[20];
        snprintf(buf, sizeof(buf), "%d device(s)", lock.device_count);
        spr.drawString(buf, 130, 226, 2);
    }

    // 帧缓冲推送到屏幕 (一次性刷新, 无闪烁)
    spr.pushSprite(0, 0);
}

// ============ Web Handlers ============
void handle_root() {
    server.send(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handle_status() {
    String json = "{";
    json += "\"ip\":\"" + wifi_ip + "\",";
    json += "\"wifi\":\"" + String(WiFi.RSSI()) + " dBm\",";
    json += "\"locked\":" + String(unlocked ? "false" : "true") + ",";
    json += "\"device\":\"";
    if (lock.device_count > 0) {
        json += lock.devices[lock.active_idx].name;
    } else {
        json += "未配置";
    }
    json += "\",";
    json += "\"result\":\"" + lock.last_result + "\",";
    json += "\"token_preview\":\"";
    String t = lock.user_token.length() > 0 ? lock.user_token : LOCK_TOKEN;
    if (t.length() > 8) json += t.substring(t.length() - 8);
    else json += t;
    json += "\"";
    json += "}";
    server.send(200, "application/json", json);
}

void handle_unlock() {
    if (lock.device_count == 0) {
        lock.last_result = "无可用门锁";
        String json = "{\"ok\":false,\"msg\":\"无可用门锁\"}";
        server.send(200, "application/json", json);
        update_display(true);
        return;
    }

    // BLE 开锁 (阻塞, 几秒钟)
    // 注意: BLE 只初始化一次, 不反复 deinit/init, 防止崩溃
    lock.init_ble();
    bool ok = lock.open_lock();

    if (ok) { unlocked = true; unlock_time = millis(); }

    String json = "{\"ok\":" + String(ok ? "true" : "false") +
                  ",\"msg\":\"" + lock.last_result + "\"}";
    server.send(200, "application/json", json);
    update_display(true);
}

void handle_refresh() {
    bool ok = lock.fetch_credentials();

    String json = "{\"ok\":" + String(ok ? "true" : "false") + "}";
    server.send(200, "application/json", json);
    update_display(true);
}

void handle_settings() {
    server.send(200, "text/html; charset=utf-8", SETTINGS_HTML);
}

void handle_save_token() {
    if (!server.hasArg("token")) {
        server.send(400, "application/json", "{\"ok\":false}");
        return;
    }
    String new_token = server.arg("token");
    new_token.trim();
    if (new_token.length() < 4) {
        server.send(400, "application/json", "{\"ok\":false,\"msg\":\"token too short\"}");
        return;
    }

    // 保存到 NVS
    {
        Preferences prefs;
        prefs.begin("ble_lock", false);
        prefs.putString("token", new_token);
        prefs.end();
    }

    // 应用到运行态
    lock.set_token(new_token);

    String json = "{\"ok\":true,\"msg\":\"token saved, re-fetching locks...\"}";
    server.send(200, "application/json", json);

    // 触发重新获取门锁
    lock.fetch_credentials();
    update_display(true);
}

void handle_notfound() {
    server.send(404, "text/plain", "404");
}

// ============ Setup ============
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== BLE Lock ===");

    // ---- TFT + 帧缓冲 ----
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    spr.createSprite(TFT_WIDTH, TFT_HEIGHT);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Booting...", 20, 100, 4);

    // ---- WiFi ----
    tft.drawString("Connecting WiFi...", 20, 140, 2);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 40) {
        delay(500);
        Serial.print(".");
        tries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        wifi_ip = WiFi.localIP().toString();
        Serial.printf("\nWiFi OK: %s\n", wifi_ip.c_str());
        tft.drawString("WiFi OK: " + wifi_ip, 20, 140, 2);
    } else {
        Serial.println("\nWiFi FAIL");
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawString("WiFi FAIL!", 20, 140, 2);
    }

    // ---- Preferences: 读取持久化的 token ----
    {
        Preferences prefs;
        prefs.begin("ble_lock", true);
        String saved = prefs.getString("token", "");
        prefs.end();
        if (saved.length() > 0) {
            lock.set_token(saved);
            Serial.println("[CFG] token loaded from NVS");
        } else {
            Serial.println("[CFG] using compile-time token");
        }
    }

    // ---- NTP ----
    sync_time();

    // ---- 获取门锁凭据 ----
    if (WiFi.status() == WL_CONNECTED) {
        tft.drawString("Fetching locks...", 20, 162, 2);
        lock.fetch_credentials();
        if (lock.device_count > 0) {
            Serial.printf("Found %d device(s)\n", lock.device_count);
        }
    }

    // ---- Web Server ----
    server.on("/", handle_root);
    server.on("/unlock", handle_unlock);
    server.on("/api/status", handle_status);
    server.on("/refresh", handle_refresh);
    server.on("/settings", handle_settings);
    server.on("/api/save_token", HTTP_POST, handle_save_token);
    server.onNotFound(handle_notfound);
    server.begin();
    Serial.println("HTTP server started");

    // ---- 注册状态/空闲回调 ----
    lock.on_status = status_callback;                     // TFT 刷新
    lock.on_idle = []{ server.handleClient(); };          // 长操作中让 WebServer 响应轮询

    // ---- 完成 ----
    update_display(true);
    Serial.println("Ready!");
}

// ============ Loop ============
void loop() {
    server.handleClient();

    // 开锁 3 秒后自动回锁 (带倒计时)
    if (unlocked) {
        unsigned long elapsed = millis() - unlock_time;
        if (elapsed > 5000) {
            // 回锁完成, 清状态回到初始界面
            unlocked = false;
            lock.locked = true;
            lock.last_result = "";
            update_display(true);
        } else if (elapsed > 3000) {
            // 3秒后已回锁, 再显示 2 秒 "relocked" 然后清除
            lock.set_status("relocked");
        } else {
            // 倒计时 3-2-1
            static unsigned long last_countdown = 0;
            int sec = 3 - elapsed / 1000;
            if (sec > 0 && (sec != last_countdown || elapsed < 500)) {
                last_countdown = sec;
                lock.set_status("unlocked, relock in " + String(sec) + "s");
            }
        }
    }

    update_display();
}
