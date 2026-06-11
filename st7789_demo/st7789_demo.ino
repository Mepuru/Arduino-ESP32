/*
 * ESP32-S3 + ST7789 — 纯原生 SPI 驱动
 * 不依赖 TFT_eSPI 或其他第三方库
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

static void dc(uint8_t v) { digitalWrite(DC, v); }
static void cs(uint8_t v) { digitalWrite(CS, v); }

static void wcmd(uint8_t c) { dc(0); cs(0); SPI.write(c); cs(1); }
static void wdat(uint8_t d) { dc(1); cs(0); SPI.write(d); cs(1); }
static void w16(uint16_t d) { dc(1); cs(0); SPI.write16(d); cs(1); }

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
  set_win(0, 0, 239, 239);
  dc(1); cs(0);
  for (int i = 0; i < 240 * 240; i++) SPI.write16(c);
  cs(1);
}

void fill_rect(int x, int y, int w, int h, uint16_t c) {
  if (w <= 0 || h <= 0) return;
  set_win(x, y, x + w - 1, y + h - 1);
  dc(1); cs(0);
  for (int i = 0; i < w * h; i++) SPI.write16(c);
  cs(1);
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("Starting...");

  st7789_init();
  Serial.println("LCD init OK");

  fill(RED);    delay(600);
  fill(GREEN);  delay(600);
  fill(BLUE);   delay(600);
  fill(BLACK);  delay(300);

  // 色条
  fill_rect(0, 0, 240, 20, RED);
  fill_rect(0, 30, 240, 20, GREEN);
  fill_rect(0, 60, 240, 20, BLUE);
  fill_rect(0, 90, 240, 20, YELLOW);
  fill_rect(0, 120, 240, 20, CYAN);
  fill_rect(0, 150, 240, 20, MAGENTA);

  delay(1000);
  fill(BLACK);
  Serial.println("Setup complete");
}

// 弹球动画
int bx = 50, by = 100, bdx = 2, bdy = 2;
int phase = 0;

void loop() {
  // 擦除旧球
  fill_rect(bx, by, 10, 10, BLACK);

  bx += bdx; by += bdy;
  if (bx <= 0 || bx >= 230) bdx = -bdx;
  if (by <= 0 || by >= 230) bdy = -bdy;

  // 画新球
  uint16_t colors[] = {RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA};
  fill_rect(bx, by, 10, 10, colors[(phase / 20) % 6]);

  // 底部状态文字（用色块拼简单形状）
  phase++;
  delay(16);
}
