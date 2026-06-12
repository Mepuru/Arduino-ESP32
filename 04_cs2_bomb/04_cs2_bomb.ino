/*
 * C4 Bomb Simulator - ESP32-S3 + ST7789 + Buzzer
 *
 * Web 页面控制炸弹下包/拆包, ST7789 显示倒计时, 蜂鸣器根据剩余时间变速报警.
 *
 * 接线:
 *   ST7789  ->  ESP32-S3
 *   VCC     ->  3.3V
 *   GND     ->  GND
 *   CS      ->  GPIO10
 *   DC      ->  GPIO5
 *   RST     ->  GPIO6
 *   SCL     ->  GPIO12
 *   SDA     ->  GPIO11
 *   BL      ->  GPIO21
 *
 *   Buzzer  ->  ESP32-S3
 *   VCC     ->  3.3V
 *   GND     ->  GND
 *   I/O     ->  GPIO4 (低电平触发)
 */

#include <WiFi.h>
#include <WebServer.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "config.h"
#include "web_ui.h"

// ============ Pin ============
#define BUZZER_PIN      4

// ============ 炸弹参数 ============
static const int TOTAL_TIME       = 45;   // 倒计时总秒数
static const int BEEP_DURATION_MS = 100;  // 每次蜂鸣持续毫秒

// ============ 状态机 ============
enum BombState { IDLE, ARMED, EXPLODED, DEFUSED };
static BombState state = IDLE;

static unsigned long arm_time     = 0;  // 下包时刻 (millis)
static unsigned long explode_time = 0;  // 爆炸时刻 (millis)
static unsigned long defuse_time  = 0;  // 拆包时刻 (millis)

// ============ 拆弹游戏 ============
static struct {
    bool   active;
    int    score;
    int    target;
    String colors[4];
    String target_color;
} game;

static const char* ALL_COLORS[] = {"red", "blue", "green", "yellow", "white", "purple"};
static const int COLOR_COUNT = 6;

// 蜂鸣器动作反馈 (剪对/剪错)
static unsigned long buzzer_fb_start = 0;
static int           buzzer_fb_duration = 0;

// ============ TFT ============
TFT_eSPI   tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// ============ Web ============
WebServer server(80);
static String wifi_ip = "";

// ============ 节流 ============
static unsigned long last_tft_update = 0;
static int           last_remaining  = -1;

// ============ 工具函数 ============

// 根据剩余秒数返回蜂鸣间隔 (ms)
static int get_beep_interval(int sec) {
    if (sec > 30) return 5000;
    if (sec > 15) return 2000;
    if (sec > 10) return 1000;
    if (sec > 5)  return 500;
    return 200;
}

// 根据剩余秒数返回进度条颜色
static uint16_t get_bar_color(int sec) {
    if (sec > 30) return TFT_WHITE;
    if (sec > 15) return TFT_YELLOW;
    if (sec > 10) return TFT_ORANGE;
    return TFT_RED;
}

// 获取当前剩余秒数
static int get_remaining() {
    if (state == IDLE) return TOTAL_TIME;
    if (state == ARMED) {
        int e = (millis() - arm_time) / 1000;
        int r = TOTAL_TIME - e;
        return (r < 0) ? 0 : r;
    }
    if (state == EXPLODED) return 0;
    if (state == DEFUSED) {
        // 冻结在拆包时的剩余时间
        int e = (defuse_time - arm_time) / 1000;
        int r = TOTAL_TIME - e;
        return (r < 0) ? 0 : r;
    }
    return TOTAL_TIME;
}

// ============ 蜂鸣器 ============
static void update_buzzer() {
    unsigned long now = millis();

    // 游戏动作反馈 (剪对短响, 剪错长响) 优先级最高
    if (buzzer_fb_duration > 0) {
        if (now - buzzer_fb_start < (unsigned long)buzzer_fb_duration) {
            digitalWrite(BUZZER_PIN, LOW);
        } else {
            digitalWrite(BUZZER_PIN, HIGH);
            buzzer_fb_duration = 0;
        }
        return;
    }

    if (state == ARMED) {
        int remaining = get_remaining();
        if (remaining <= 0) {
            state = EXPLODED;
            game.active = false;
            explode_time = now;
            digitalWrite(BUZZER_PIN, HIGH);
            return;
        }
        int interval = get_beep_interval(remaining);
        int phase = (now - arm_time) % interval;
        digitalWrite(BUZZER_PIN, (phase < BEEP_DURATION_MS) ? LOW : HIGH);

    } else if (state == EXPLODED) {
        digitalWrite(BUZZER_PIN, (now - explode_time < 3000) ? LOW : HIGH);

    } else {
        digitalWrite(BUZZER_PIN, HIGH);
    }
}

// ============ 显示屏 ============
static void update_display(bool force = false) {
    unsigned long now = millis();
    if (!force && now - last_tft_update < 200) return;
    last_tft_update = now;

    int remaining = get_remaining();

    spr.fillSprite(TFT_BLACK);

    // ---- 标题 ----
    spr.setTextColor(TFT_CYAN, TFT_BLACK);
    spr.drawString("C4 BOMB", 8, 4, 4);

    // ---- 状态文字 ----
    const char* status_str = "";
    uint16_t    status_color = TFT_WHITE;
    switch (state) {
        case IDLE:     status_str = "IDLE";     status_color = TFT_DARKGREY; break;
        case ARMED:    status_str = "ARMED";    status_color = TFT_RED;      break;
        case EXPLODED: status_str = "EXPLODED"; status_color = TFT_RED;      break;
        case DEFUSED:  status_str = "DEFUSED";  status_color = TFT_GREEN;    break;
    }
    spr.setTextColor(status_color, TFT_BLACK);
    spr.drawString(status_str, 8, 32, 2);

    // ---- 倒计时 (大号 7 段数码管) ----
    if (state != IDLE) {
        char buf[10];
        int m = remaining / 60;
        int s = remaining % 60;
        snprintf(buf, sizeof(buf), "%d:%02d", m, s);

        uint16_t timer_color = TFT_RED;
        if (state == DEFUSED) timer_color = TFT_GREEN;
        spr.setTextColor(timer_color, TFT_BLACK);
        spr.drawString(buf, 8, 52, 7);  // font 7 = 48px 7-segment
    }

    // ---- 进度条 ----
    if (state != IDLE) {
        int bar_x = 10, bar_y = 110, bar_w = 220, bar_h = 10;
        int fill_w = (remaining * bar_w) / TOTAL_TIME;
        uint16_t bcol = (state == DEFUSED) ? TFT_GREEN : get_bar_color(remaining);

        spr.drawRoundRect(bar_x, bar_y, bar_w, bar_h, 3, TFT_DARKGREY);
        if (fill_w > 0) {
            spr.fillRoundRect(bar_x, bar_y, fill_w, bar_h, 3, bcol);
        }
    }

    // ---- 结果/提示信息 ----
    if (state == EXPLODED) {
        spr.setTextColor(TFT_RED, TFT_BLACK);
        spr.drawString("EXPLODED", 8, 130, 4);
    } else if (state == DEFUSED) {
        spr.setTextColor(TFT_GREEN, TFT_BLACK);
        spr.drawString("DEFUSED", 8, 130, 4);
    } else if (state == IDLE) {
        spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
        spr.drawString("Open browser to control", 8, 130, 2);
    } else if (state == ARMED && game.active) {
        spr.setTextColor(TFT_CYAN, TFT_BLACK);
        spr.drawString("GAME", 8, 130, 2);
        char sb[24];
        snprintf(sb, sizeof(sb), "Score: %d/%d", game.score, game.target);
        spr.setTextColor(TFT_WHITE, TFT_BLACK);
        spr.drawString(sb, 8, 148, 2);
    }

    // ---- IP ----
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString(wifi_ip, 8, 224, 2);

    spr.pushSprite(0, 0);
}

// ============ Web 页面 (web_ui.h) ============

// ============ Web Handlers ============
static void handle_root() {
    server.send(200, "text/html; charset=utf-8", INDEX_HTML);
}

static void handle_status() {
    int remaining = get_remaining();
    String json = "{";
    json += "\"state\":\"";
    switch (state) {
        case IDLE:     json += "IDLE";     break;
        case ARMED:    json += "ARMED";    break;
        case EXPLODED: json += "EXPLODED"; break;
        case DEFUSED:  json += "DEFUSED";  break;
    }
    json += "\",";
    json += "\"remaining\":" + String(remaining) + ",";
    json += "\"total\":" + String(TOTAL_TIME);
    json += "}";
    server.send(200, "application/json", json);
}

static void handle_plant() {
    if (state != IDLE) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"Already armed or exploded\"}");
        return;
    }
    String pw = server.arg("password");
    if (pw != PLANT_PASSWORD) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"Incorrect password\"}");
        return;
    }

    game.active = false;
    game.score = 0;
    state = ARMED;
    arm_time = millis();
    update_display(true);
    server.send(200, "application/json", "{\"ok\":true,\"msg\":\"Bomb planted\"}");
}

static void handle_defuse() {
    if (state != ARMED) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"No bomb to defuse\"}");
        return;
    }
    String pw = server.arg("password");
    if (pw != DEFUSE_PASSWORD) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"Incorrect password\"}");
        return;
    }

    state = DEFUSED;
    defuse_time = millis();
    digitalWrite(BUZZER_PIN, HIGH);  // 立即静音
    update_display(true);
    server.send(200, "application/json", "{\"ok\":true,\"msg\":\"Bomb defused\"}");
}

static void handle_reset() {
    state = IDLE;
    game.active = false;
    game.score = 0;
    arm_time = 0;
    explode_time = 0;
    defuse_time = 0;
    buzzer_fb_duration = 0;
    digitalWrite(BUZZER_PIN, HIGH);
    update_display(true);
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_notfound() {
    server.send(404, "text/plain", "404");
}

// ============ 拆弹游戏 Handlers ============

static String game_state_json() {
    String j = "{\"ok\":true,\"colors\":[";
    for (int i = 0; i < COLOR_COUNT; i++) {
        if (i > 0) j += ",";
        j += "\"" + String(ALL_COLORS[i]) + "\"";
    }
    j += "],\"target_color\":\"" + game.target_color + "\",";
    j += "\"score\":" + String(game.score) + ",";
    j += "\"target\":" + String(game.target) + "}";
    return j;
}

static void handle_game_start() {
    if (state != ARMED) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"Not in ARMED state\"}");
        return;
    }
    if (game.active) {
        server.send(200, "application/json", game_state_json());
        return;
    }
    game.active = true;
    game.score = 0;
    game.target = 7;
    int idx = random(COLOR_COUNT);
    game.target_color = ALL_COLORS[idx];

    update_display(true);
    server.send(200, "application/json", game_state_json());
}

static void handle_game_cut() {
    if (!game.active || state != ARMED) {
        server.send(200, "application/json", "{\"ok\":false,\"msg\":\"No active game\"}");
        return;
    }

    String color = server.arg("color");
    bool timeout = (color == "timeout");
    bool correct = !timeout && (color == game.target_color);

    // 蜂鸣器反馈: 剪对短响, 剪错/超时都长响
    buzzer_fb_duration = correct ? 80 : 500;
    buzzer_fb_start = millis();

    String j = "{\"ok\":true,\"correct\":" + String(correct ? "true" : "false");

    if (correct) {
        game.score++;
        if (game.score >= game.target) {
            state = DEFUSED;
            defuse_time = millis();
            game.active = false;
            digitalWrite(BUZZER_PIN, HIGH);
            j += ",\"score\":" + String(game.score);
            j += ",\"target\":" + String(game.target);
            j += ",\"state\":\"DEFUSED\"";
            j += ",\"msg\":\"Bomb defused!\"";
            j += "}";
            update_display(true);
            server.send(200, "application/json", j);
            return;
        }
        {
            int idx = random(COLOR_COUNT);
            game.target_color = ALL_COLORS[idx];
            j += ",\"score\":" + String(game.score);
            j += ",\"target\":" + String(game.target);
            String colors_json = "[";
            for (int i = 0; i < COLOR_COUNT; i++) {
                if (i > 0) colors_json += ",";
                colors_json += "\"" + String(ALL_COLORS[i]) + "\"";
            }
            colors_json += "]";
            j += ",\"colors\":" + colors_json;
            j += ",\"target_color\":\"" + game.target_color + "\"";
            j += ",\"msg\":\"Correct!\",\"state\":\"ARMED\"}";
        }
    } else {
        game.score = 0;
        int idx = random(COLOR_COUNT);
        game.target_color = ALL_COLORS[idx];
        j += ",\"score\":0";
        j += ",\"target\":" + String(game.target);
        String colors_json = "[";
        for (int i = 0; i < COLOR_COUNT; i++) {
            if (i > 0) colors_json += ",";
            colors_json += "\"" + String(ALL_COLORS[i]) + "\"";
        }
        colors_json += "]";
        j += ",\"colors\":" + colors_json;
        j += ",\"target_color\":\"" + game.target_color + "\"";
        j += ",\"msg\":\"" + String(timeout ? "Too slow!" : "Wrong wire!") + "\"";
        j += ",\"state\":\"ARMED\"";
        j += "}";
    }

    update_display(true);
    server.send(200, "application/json", j);
}

static void handle_game_abort() {
    game.active = false;
    game.score = 0;
    buzzer_fb_duration = 0;
    update_display(true);
    server.send(200, "application/json", "{\"ok\":true}");
}

// ============ Setup ============
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== C4 Bomb ===");

    // ---- 随机种子 ----
    randomSeed(esp_random());

    // ---- 蜂鸣器 ----
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH);  // 初始静音

    // ---- TFT ----
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    spr.createSprite(TFT_WIDTH, TFT_HEIGHT);

    spr.fillSprite(TFT_BLACK);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawString("C4 BOMB", 20, 80, 4);
    spr.drawString("Booting...", 20, 120, 2);
    spr.pushSprite(0, 0);

    // ---- WiFi ----
    spr.drawString("Connecting WiFi...", 20, 140, 2);
    spr.pushSprite(0, 0);
    Serial.print("Connecting WiFi");

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
        spr.fillSprite(TFT_BLACK);
        spr.setTextColor(TFT_GREEN, TFT_BLACK);
        spr.drawString("WiFi OK", 20, 120, 2);
        spr.drawString(wifi_ip, 20, 140, 2);
        spr.pushSprite(0, 0);
    } else {
        Serial.println("\nWiFi FAIL");
        spr.fillSprite(TFT_BLACK);
        spr.setTextColor(TFT_RED, TFT_BLACK);
        spr.drawString("WiFi FAIL!", 20, 120, 2);
        spr.pushSprite(0, 0);
        wifi_ip = "No WiFi";
    }

    // ---- Web Server ----
    server.on("/", handle_root);
    server.on("/api/status", handle_status);
    server.on("/api/plant", HTTP_POST, handle_plant);
    server.on("/api/defuse", HTTP_POST, handle_defuse);
    server.on("/api/reset", HTTP_POST, handle_reset);
    server.on("/api/game/start", HTTP_POST, handle_game_start);
    server.on("/api/game/cut",   HTTP_POST, handle_game_cut);
    server.on("/api/game/abort", HTTP_POST, handle_game_abort);
    server.onNotFound(handle_notfound);
    server.begin();
    Serial.println("HTTP server started");

    // ---- 完成 ----
    delay(500);
    update_display(true);
    Serial.println("Ready!");
}

// ============ Loop ============
void loop() {
    server.handleClient();
    update_buzzer();
    update_display();
}
