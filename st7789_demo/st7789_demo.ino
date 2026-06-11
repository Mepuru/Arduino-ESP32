/*
 * ESP32-S3 + ST7789
 * 手动初始化 SPI + ST7789，然后使用 TFT_eSPI 的绘图功能
 * 编译选项: 启用 PSRAM (OPI PSRAM)
 */
#include <TFT_eSPI.h>
#include <SPI.h>

// 在 setup() 内创建对象，避免全局静态初始化顺序问题
TFT_eSPI* tft = nullptr;

// === 手动 ST7789 初始化（已验证可行）===
static void spi_tx(const uint8_t* data, size_t len) {
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE0));
  for (size_t i = 0; i < len; i++) SPI.write(data[i]);
  SPI.endTransaction();
}

static void wcmd(uint8_t c) {
  digitalWrite(TFT_DC, LOW);
  digitalWrite(TFT_CS, LOW);
  SPI.write(c);
  digitalWrite(TFT_CS, HIGH);
}

static void wdat(uint8_t d) {
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  SPI.write(d);
  digitalWrite(TFT_CS, HIGH);
}

static void w16(uint16_t d) {
  uint8_t buf[2] = { (uint8_t)(d >> 8), (uint8_t)(d & 0xFF) };
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, LOW);
  SPI.writeBytes(buf, 2);
  digitalWrite(TFT_CS, HIGH);
}

void manual_init_display() {
  pinMode(TFT_CS, OUTPUT); pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_RST, OUTPUT); pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  digitalWrite(TFT_CS, HIGH); digitalWrite(TFT_DC, HIGH);

  digitalWrite(TFT_RST, LOW); delay(10);
  digitalWrite(TFT_RST, HIGH); delay(120);

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  wcmd(0x01); delay(150);
  wcmd(0x11); delay(150);
  wcmd(0x36); wdat(0);
  wcmd(0x3A); wdat(0x55);
  wcmd(0x21); delay(10);
  wcmd(0x13); delay(10);
  wcmd(0x29); delay(10);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("Starting...");

  // 手动初始化（绕过 TFT_eSPI 的 bug）
  manual_init_display();
  Serial.println("Manual init OK");

  // 现在创建 TFT_eSPI 对象并使用其绘图功能
  // 不调用 tft->init()，因为屏幕已经初始化好了
  tft = new TFT_eSPI();
  Serial.println("TFT_eSPI object created");

  // 设置旋转方向
  tft->setRotation(1);

  // 用 TFT_eSPI 的绘图 API
  Serial.println("RED...");
  tft->fillScreen(TFT_RED);
  delay(1500);

  Serial.println("GREEN...");
  tft->fillScreen(TFT_GREEN);
  delay(1500);

  Serial.println("BLUE...");
  tft->fillScreen(TFT_BLUE);
  delay(1500);

  // 绘制图形
  tft->fillScreen(TFT_BLACK);
  tft->fillRect(10, 10, 100, 60, TFT_RED);
  tft->fillRect(130, 10, 100, 60, TFT_GREEN);
  tft->fillRect(10, 90, 100, 60, TFT_BLUE);
  tft->fillRect(130, 90, 100, 60, TFT_YELLOW);
  tft->fillCircle(60, 200, 25, TFT_MAGENTA);
  tft->fillCircle(180, 200, 25, TFT_CYAN);
  delay(2000);

  // 文字
  tft->fillScreen(TFT_BLACK);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->drawString("ESP32-S3 + ST7789", 20, 20, 4);
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
  tft->drawString("TFT_eSPI drawing", 20, 60, 4);
  tft->setTextColor(TFT_CYAN, TFT_BLACK);
  tft->drawString("240x240 16-bit", 20, 100, 2);
  tft->setTextColor(TFT_YELLOW, TFT_BLACK);
  tft->drawString("Manual init + TFT_eSPI", 20, 130, 2);
  delay(2000);

  Serial.println("Setup complete");
}

void loop() {
  // 弹球动画
  static int bx = 50, by = 50, dx = 2, dy = 3;

  tft->fillCircle(bx, by, 10, TFT_BLACK);
  bx += dx; by += dy;
  if (bx <= 5 || bx >= 235) dx = -dx;
  if (by <= 5 || by >= 235) dy = -dy;

  static int ci = 0;
  uint16_t colors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};
  tft->fillCircle(bx, by, 10, colors[ci]);
  ci = (ci + 1) % 6;

  delay(20);
}
