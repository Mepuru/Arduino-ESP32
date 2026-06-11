/*
 * ESP32-S3 + ST7789 诊断测试 — 逐步验证
 */

#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n=== ST7789 DIAG ===\n");

  // 步骤 1: 引脚配置
  Serial.printf("STEP 1: TFT_CS=%d, TFT_DC=%d, TFT_RST=%d\n", TFT_CS, TFT_DC, TFT_RST);
  Serial.printf("STEP 1: TFT_MOSI=%d, TFT_SCLK=%d, TFT_BL=%d\n", TFT_MOSI, TFT_SCLK, TFT_BL);
  Serial.println("STEP 1: OK");

  // 步骤 2: 背光
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  Serial.println("STEP 2: Backlight ON - OK");

  // 步骤 3: 手动复位
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  delay(20);
  digitalWrite(TFT_RST, HIGH);
  delay(150);
  Serial.println("STEP 3: RST toggled - OK");

  // 步骤 4: 初始化
  Serial.println("STEP 4: Calling tft.init()...");
  Serial.flush();
  tft.init();
  Serial.println("STEP 4: tft.init() done - OK");

  // 步骤 5: 旋转
  tft.setRotation(0);
  Serial.println("STEP 5: Rotation set - OK");

  // 步骤 6: 纯色测试
  Serial.println("STEP 6: RED...");
  tft.fillScreen(TFT_RED);
  delay(2000);

  Serial.println("STEP 6: GREEN...");
  tft.fillScreen(TFT_GREEN);
  delay(2000);

  Serial.println("STEP 6: BLUE...");
  tft.fillScreen(TFT_BLUE);
  delay(2000);

  tft.fillScreen(TFT_BLACK);
  Serial.println("STEP 6: Colors done - OK");

  Serial.println("\n=== DIAG COMPLETE ===");
}

void loop() {
  Serial.println("loop tick");
  delay(3000);
}
