/*
 * ESP32-S3 + ST7789 — 混合驱动
 * SPI 初始化和屏幕初始化用手动方式（已验证可行）
 * 图形绘制用 TFT_eSPI（跳过它的 init() 崩溃）
 */
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// === 手动 SPI 命令函数（绕开 TFT_eSPI 的 init）===
static void raw_cmd(uint8_t c) {
  digitalWrite(TFT_DC, LOW);
  digitalWrite(TFT_CS, LOW);
  SPI.write(c);
  while (SPI.getClockDivider());
  digitalWrite(TFT_CS, HIGH);
}

static void raw_data(uint8_t d) {
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  SPI.write(d);
  while (SPI.getClockDivider());
  digitalWrite(TFT_CS, HIGH);
}

static void raw_data16(uint16_t d) {
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  SPI.write16(d);
  while (SPI.getClockDivider());
  digitalWrite(TFT_CS, HIGH);
}

// === 手动初始化 ST7789（已验证可行）===
void manual_st7789_init() {
  pinMode(TFT_RST, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_BL, OUTPUT);

  digitalWrite(TFT_BL, HIGH);
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TFT_DC, HIGH);

  // Reset
  digitalWrite(TFT_RST, LOW);
  delay(10);
  digitalWrite(TFT_RST, HIGH);
  delay(120);

  // ST7789 init sequence
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  raw_cmd(0x01);  delay(150);  // SWRESET
  raw_cmd(0x11);  delay(150);  // SLPOUT
  raw_cmd(0x36);  raw_data(0x00);  // MADCTL
  raw_cmd(0x3A);  raw_data(0x55);  // COLMOD (16-bit)
  raw_cmd(0x13);  delay(10);      // NORON
  raw_cmd(0x29);  delay(10);      // DISPON

  Serial.println("Manual ST7789 init done");
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== Starting ===\n");

  manual_st7789_init();

  // 现在用 TFT_eSPI 画图（不调用 tft.init()）
  Serial.println("Drawing with TFT_eSPI...");

  tft.fillScreen(TFT_RED);
  delay(1000);

  tft.fillScreen(TFT_GREEN);
  delay(1000);

  tft.fillScreen(TFT_BLUE);
  delay(1000);

  tft.fillScreen(TFT_BLACK);
  delay(500);

  // Demo
  tft.setRotation(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("ESP32-S3 + ST7789", 120, 80, 4);
  tft.drawCentreString("Manual init works!", 120, 120, 2);
  tft.drawCentreString("TFT_eSPI drawing OK", 120, 150, 2);

  Serial.println("Setup complete");
}

void loop() {
  static unsigned long t = 0;
  t++;
  tft.drawPixel(random(240), random(240), random(65536));
  delay(10);
}
