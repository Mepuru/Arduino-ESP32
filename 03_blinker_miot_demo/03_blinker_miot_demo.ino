/*
 * Blinker 远程控制演示 + 米家 MIOT - ESP32-S3 + ST7789
 *
 * Blinker App 4 个按钮 + 米家开关双重控制。
 * 米家功能依赖 Blinker 云端桥接，官方已宣布 2025.7.7 停止服务，实测已不可用。
 *
 * Blinker App 编辑界面添加按键:
 *   btn-power, btn-color, btn-demo, btn-reset
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
 */

#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT

#include <Blinker.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include "config.h"

// ============ 全局 ============
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

static bool        power_on     = false;
static int         toggle_count = 0;
static int         display_mode = 0;     // 0=状态, 1=图形, 2=弹球
static uint16_t    accent_color = TFT_GREEN;
static unsigned long last_tft_update = 0;
static String      wifi_ip = "";

// 弹球参数
static float ball_x = 120, ball_y = 120;
static float ball_vx = 2.5, ball_vy = 1.8;
static int   ball_r = 8;

// 4 个 Blinker 按键
BlinkerButton BtnPower("btn-power");
BlinkerButton BtnColor("btn-color");
BlinkerButton BtnDemo("btn-demo");
BlinkerButton BtnReset("btn-reset");

// ============ 主题色 ============
static const uint16_t color_palette[] = {
    TFT_GREEN, TFT_CYAN, TFT_YELLOW, TFT_MAGENTA, TFT_ORANGE, TFT_WHITE
};
static const int color_count = sizeof(color_palette) / sizeof(color_palette[0]);
static int color_index = 0;

void next_color() {
    color_index = (color_index + 1) % color_count;
    accent_color = color_palette[color_index];
}

// ============ 显示模式 0: 状态屏 ============
void draw_status_screen() {
    spr.fillSprite(TFT_BLACK);

    spr.setTextColor(TFT_CYAN, TFT_BLACK);
    spr.drawString("Blinker Demo", 8, 4, 4);

    spr.setTextColor(TFT_GREEN, TFT_BLACK);
    spr.drawString(wifi_ip, 8, 36, 2);

    int rssi = WiFi.RSSI();
    spr.setTextColor(rssi > -70 ? TFT_GREEN : TFT_YELLOW, TFT_BLACK);
    spr.drawString("WiFi: " + String(rssi) + " dBm", 8, 58, 2);

    spr.setTextColor(Blinker.connected() ? TFT_GREEN : TFT_RED, TFT_BLACK);
    spr.drawString(Blinker.connected() ? "Blinker: Connected" : "Blinker: Disconnected",
                   8, 80, 2);

    spr.drawFastHLine(0, 108, 240, TFT_DARKGREY);

    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(power_on ? accent_color : TFT_RED, TFT_BLACK);
    spr.drawString(power_on ? "● ON" : "○ OFF", 8, 144, 6);
    spr.setTextDatum(TL_DATUM);

    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawString("Toggles: " + String(toggle_count), 8, 178, 2);

    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString("Mode: " + String(display_mode + 1) + "/3", 8, 200, 2);

    spr.fillRect(0, 224, 240, 16, power_on ? accent_color : TFT_MAROON);
    spr.setTextColor(TFT_WHITE, spr.color16to8(power_on ? accent_color : TFT_MAROON));
    spr.drawString(power_on ? "POWER ON" : "POWER OFF", 8, 226, 2);
}

// ============ 显示模式 1: 图形屏 ============
void draw_graphics_screen() {
    spr.fillSprite(TFT_BLACK);

    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawString("Graphics Demo", 8, 4, 4);

    spr.fillCircle(60, 80, 30, TFT_RED);
    spr.fillRect(130, 60, 50, 50, TFT_BLUE);
    spr.fillTriangle(120, 160, 60, 200, 180, 200, TFT_YELLOW);
    spr.drawRoundRect(10, 180, 60, 40, 8, TFT_GREEN);
    spr.drawEllipse(200, 80, 30, 20, TFT_CYAN);

    spr.fillCircle(12, 12, 6, power_on ? accent_color : TFT_RED);
    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString("T:" + String(toggle_count), 160, 224, 2);
}

// ============ 显示模式 2: 弹球屏 ============
void draw_bounce_screen() {
    spr.fillSprite(TFT_BLACK);

    spr.drawRect(0, 0, 240, 240, accent_color);

    ball_x += ball_vx;
    ball_y += ball_vy;

    if (ball_x - ball_r <= 0 || ball_x + ball_r >= 240) ball_vx = -ball_vx;
    if (ball_y - ball_r <= 0 || ball_y + ball_r >= 240) ball_vy = -ball_vy;

    spr.fillCircle((int)ball_x, (int)ball_y, ball_r, accent_color);
    spr.fillCircle((int)ball_x, (int)ball_y, ball_r - 3, TFT_WHITE);

    spr.setTextDatum(CC_DATUM);
    spr.setTextColor(accent_color, TFT_WHITE);
    spr.drawString(String(toggle_count), (int)ball_x, (int)ball_y, 2);
    spr.setTextDatum(TL_DATUM);

    spr.setTextColor(TFT_DARKGREY, TFT_BLACK);
    spr.drawString("Bounce", 8, 224, 2);
}

// ============ 统一刷新 ============
void update_display(bool force = false) {
    if (!force && millis() - last_tft_update < 50) return;
    last_tft_update = millis();

    switch (display_mode) {
        case 0:  draw_status_screen();   break;
        case 1:  draw_graphics_screen(); break;
        case 2:  draw_bounce_screen();   break;
        default: draw_status_screen();   break;
    }

    spr.pushSprite(0, 0);
}

// ============ 公共: 切换电源状态 ============
void set_power(bool on) {
    if (power_on == on) return;
    power_on = on;
    toggle_count++;
    update_display(true);
}

void toggle_power() {
    set_power(!power_on);
}

// ============ 米家 MIOT 回调 ============
void miotPowerState(const String & state) {
    BLINKER_LOG("MIOT power state: ", state);

    if (state == BLINKER_CMD_ON) {
        set_power(true);
    } else if (state == BLINKER_CMD_OFF) {
        set_power(false);
    }

    BlinkerMIOT.powerState(power_on ? "on" : "off");
    BlinkerMIOT.print();
}

void miotColor(int32_t color) {
    BLINKER_LOG("MIOT color: ", color);
    BlinkerMIOT.color(color);
    BlinkerMIOT.print();
}

void miotMode(uint8_t mode) {
    BLINKER_LOG("MIOT mode: ", mode);
    BlinkerMIOT.mode(mode);
    BlinkerMIOT.print();
}

void miotBright(const String & bright) {
    BLINKER_LOG("MIOT brightness: ", bright);
    BlinkerMIOT.brightness(bright.toInt());
    BlinkerMIOT.print();
}

void miotColoTemp(int32_t colorTemp) {
    BLINKER_LOG("MIOT colorTemp: ", colorTemp);
    BlinkerMIOT.colorTemp(colorTemp);
    BlinkerMIOT.print();
}

void miotQuery(int32_t queryCode) {
    BLINKER_LOG("MIOT Query: ", queryCode);

    BlinkerMIOT.powerState(power_on ? "on" : "off");
    BlinkerMIOT.color(0);
    BlinkerMIOT.mode(0);
    BlinkerMIOT.colorTemp(1000);
    BlinkerMIOT.brightness(1);
    BlinkerMIOT.print();
}

// ============ Blinker 按键回调 ============
void btn_power_callback(const String & state) {
    if (state != BLINKER_CMD_BUTTON_TAP) return;

    toggle_power();

    // 同步回传米家状态
    BlinkerMIOT.powerState(power_on ? "on" : "off");
    BlinkerMIOT.print();

    BtnPower.text(power_on ? "ON" : "OFF");
    BtnPower.print();
}

void btn_color_callback(const String & state) {
    if (state != BLINKER_CMD_BUTTON_TAP) return;
    next_color();
    update_display(true);
    BtnColor.text("Color");
    BtnColor.print();
}

void btn_demo_callback(const String & state) {
    if (state != BLINKER_CMD_BUTTON_TAP) return;

    display_mode = (display_mode + 1) % 3;
    if (display_mode == 2) {
        ball_x = 120; ball_y = 120;
    }
    update_display(true);

    static const char* mode_names[] = {"Status", "Graphics", "Bounce"};
    BtnDemo.text(mode_names[display_mode]);
    BtnDemo.print();
}

void btn_reset_callback(const String & state) {
    if (state != BLINKER_CMD_BUTTON_TAP) return;

    toggle_count = 0;
    power_on = false;
    display_mode = 0;
    color_index = 0;
    accent_color = TFT_GREEN;
    ball_x = 120; ball_y = 120;
    update_display(true);

    // 同步米家
    BlinkerMIOT.powerState("off");
    BlinkerMIOT.print();

    BtnReset.text("Reset OK");
    BtnReset.print();
}

// ============ Setup ============
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Blinker Demo + MIOT ===");

    // ---- TFT ----
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    spr.createSprite(TFT_WIDTH, TFT_HEIGHT);
    spr.setTextColor(TFT_WHITE, TFT_BLACK);
    spr.drawString("Booting...", 20, 100, 4);
    spr.pushSprite(0, 0);

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

    // ---- Blinker ----
    tft.drawString("Connecting Blinker...", 20, 162, 2);
    Blinker.begin(BLINKER_AUTH, WIFI_SSID, WIFI_PASS);

    // 注册米家 MIOT 回调
    BlinkerMIOT.attachPowerState(miotPowerState);
    BlinkerMIOT.attachColor(miotColor);
    BlinkerMIOT.attachMode(miotMode);
    BlinkerMIOT.attachBrightness(miotBright);
    BlinkerMIOT.attachColorTemperature(miotColoTemp);
    BlinkerMIOT.attachQuery(miotQuery);

    // 注册 Blinker App 按键回调
    BtnPower.attach(btn_power_callback);
    BtnColor.attach(btn_color_callback);
    BtnDemo.attach(btn_demo_callback);
    BtnReset.attach(btn_reset_callback);

    // ---- 完成 ----
    tft.drawString("Ready!", 20, 184, 2);
    update_display(true);
    Serial.println("Ready!");
}

// ============ Loop ============
void loop() {
    Blinker.run();
    update_display();
}
