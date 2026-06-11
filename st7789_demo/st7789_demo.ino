/*
 * ESP32-S3 + ST7789 — 已修复 TFT_eSPI
 * 改了全局库 TFT_eSPI.cpp 第647行:
 *   spi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
 *   原为 -1，ESP32-S3 上会崩溃
 */
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>

TFT_eSPI tft = TFT_eSPI();

static const uint16_t rainbow[] = {
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN,
  TFT_CYAN, TFT_BLUE, TFT_MAGENTA
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nStarting...");

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.init();
  Serial.println("tft.init() OK");

  tft.setRotation(1);
  tft.fillScreen(TFT_RED);   delay(1000);
  tft.fillScreen(TFT_GREEN); delay(1000);
  tft.fillScreen(TFT_BLUE);  delay(1000);
  tft.fillScreen(TFT_BLACK); delay(500);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("ESP32-S3 + ST7789", 20, 30, 4);
  tft.drawString("TFT_eSPI fixed!", 20, 70, 4);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("One line changed:", 20, 120, 2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("-1  ->  TFT_CS", 20, 145, 2);
  delay(2000);

  Serial.println("Setup done");
}

void loop() {
  static int x = 10, y = 10, dx = 2, dy = 3, c = 0;
  tft.fillCircle(x, y, 8, TFT_BLACK);
  x += dx; y += dy;
  if (x < 5 || x > 234) dx = -dx;
  if (y < 5 || y > 234) dy = -dy;
  tft.fillCircle(x, y, 8, rainbow[(c++ / 10) % 7]);
  delay(15);
}
