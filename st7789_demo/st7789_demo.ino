/*
 * ESP32-S3N15R8 + ST7789 240x240 显示屏演示
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
 * 配置:
 *   Arduino IDE 选 ESP32S3 Dev Module
 *   PSRAM: OPI PSRAM
 *   Flash Size: 16MB
 *
 * 关键: tft_setup.h 中定义 USE_HSPI_PORT
 *   否则 ESP32-S3 上 TFT_eSPI init 会崩溃
 */

#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>

TFT_eSPI tft = TFT_eSPI();

static const uint16_t colors[] = {
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN,
  TFT_CYAN, TFT_BLUE, TFT_MAGENTA
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP32-S3 + ST7789 starting...");

  // 背光
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // TFT_eSPI 初始化（USE_HSPI_PORT 下不会崩溃）
  tft.init();
  tft.setRotation(1);
  Serial.println("tft.init() OK");

  // === 演示 ===
  tft.fillScreen(TFT_RED);    delay(1000);
  tft.fillScreen(TFT_GREEN);  delay(1000);
  tft.fillScreen(TFT_BLUE);   delay(1000);
  tft.fillScreen(TFT_BLACK);  delay(500);

  // 彩色图形
  tft.fillRect(10, 10, 100, 60, TFT_RED);
  tft.fillRect(130, 10, 100, 60, TFT_GREEN);
  tft.fillRect(10, 90, 100, 60, TFT_BLUE);
  tft.fillRect(130, 90, 100, 60, TFT_YELLOW);
  tft.fillCircle(60, 200, 25, TFT_MAGENTA);
  tft.fillCircle(180, 200, 25, TFT_CYAN);
  delay(2000);

  // 文字
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Hello World!", 20, 20, 4);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("ESP32-S3", 20, 58, 4);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("ST7789 240x240", 20, 96, 2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("USE_HSPI_PORT fix", 20, 130, 2);
  delay(2000);

  // 系统信息
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(20, 20); tft.setTextFont(2);
  tft.println("Chip: " + String(ESP.getChipModel()));
  tft.println("Rev: v" + String(ESP.getChipRevision()));
  tft.println("Cores: " + String(ESP.getChipCores()));
  tft.println("Free Heap: " + String(ESP.getFreeHeap()/1024) + " KB");
  tft.println("PSRAM: " + String(ESP.getPsramSize()/1024/1024) + " MB");
  tft.setTextFont(1);
  tft.print("MAC: " + WiFi.macAddress());
  delay(4000);

  Serial.println("Setup complete, entering loop...");
}

void loop() {
  static int bx = 10, by = 10, dx = 2, dy = 3;
  static int ci = 0;

  tft.fillCircle(bx, by, 10, TFT_BLACK);
  bx += dx; by += dy;
  if (bx < 5 || bx > 234) dx = -dx;
  if (by < 5 || by > 234) dy = -dy;
  tft.fillCircle(bx, by, 10, colors[ci]);
  ci = (ci + 1) % 7;

  delay(15);
}
