/*
 * ESP32-S3 + ST7789 — 纯原生 SPI 驱动
 */
#include <SPI.h>

#define CS   10
#define DC   5
#define RST  6
#define MOSI 11
#define SCLK 12
#define BL   21

#define BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define WHITE   0xFFFF
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0

static SPISettings spiSettings(20000000, MSBFIRST, SPI_MODE0);

static void spi_start()  { SPI.beginTransaction(spiSettings); }
static void spi_end()    { SPI.endTransaction(); }

static void wcmd(uint8_t c) {
  digitalWrite(DC, LOW);
  digitalWrite(CS, LOW);
  SPI.write(c);
  digitalWrite(CS, HIGH);
}

static void wdat(uint8_t d) {
  digitalWrite(DC, HIGH);
  digitalWrite(CS, LOW);
  SPI.write(d);
  digitalWrite(CS, HIGH);
}

static void w16(uint16_t d) {
  digitalWrite(DC, HIGH);
  digitalWrite(CS, LOW);
  SPI.write16(d);
  digitalWrite(CS, HIGH);
}

void st7789_init() {
  pinMode(CS, OUTPUT); pinMode(DC, OUTPUT);
  pinMode(RST, OUTPUT); pinMode(BL, OUTPUT);
  digitalWrite(BL, 1); digitalWrite(CS, 1); digitalWrite(DC, 1);
  digitalWrite(RST, 0); delay(10); digitalWrite(RST, 1); delay(120);
  SPI.begin(SCLK, -1, MOSI, CS);

  wcmd(0x01); delay(150);
  wcmd(0x11); delay(150);
  wcmd(0x36); wdat(0);
  wcmd(0x3A); wdat(0x55);
  wcmd(0x21); delay(10);
  wcmd(0x13); delay(10);
  wcmd(0x29); delay(10);
}

void set_win(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  wcmd(0x2A); w16(x0); w16(x1);
  wcmd(0x2B); w16(y0); w16(y1);
}

void fill(uint16_t c) {
  spi_start();
  set_win(0, 0, 239, 239);
  wcmd(0x2C);
  digitalWrite(DC, HIGH);
  digitalWrite(CS, LOW);
  for (int i = 0; i < 240 * 240; i++) SPI.write16(c);
  digitalWrite(CS, HIGH);
  spi_end();
}

void fill_rect(int x, int y, int w, int h, uint16_t c) {
  if (w <= 0 || h <= 0) return;
  spi_start();
  set_win(x, y, x + w - 1, y + h - 1);
  wcmd(0x2C);
  digitalWrite(DC, HIGH);
  digitalWrite(CS, LOW);
  for (int i = 0; i < w * h; i++) SPI.write16(c);
  digitalWrite(CS, HIGH);
  spi_end();
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("Starting...");

  st7789_init();
  Serial.println("LCD init OK");

  Serial.println("RED...");
  fill(RED);    delay(2000);

  Serial.println("GREEN...");
  fill(GREEN);  delay(2000);

  Serial.println("BLUE...");
  fill(BLUE);   delay(2000);

  Serial.println("Cyan...");
  fill(CYAN);   delay(1500);

  Serial.println("White...");
  fill(WHITE);  delay(1500);

  Serial.println("Bars...");
  fill_rect(0, 0, 240, 48, RED);
  fill_rect(0, 48, 240, 48, GREEN);
  fill_rect(0, 96, 240, 48, BLUE);
  fill_rect(0, 144, 240, 48, YELLOW);
  fill_rect(0, 192, 240, 48, MAGENTA);
  delay(2000);

  Serial.println("Setup complete");
}

void loop() {
  delay(10000);
}
