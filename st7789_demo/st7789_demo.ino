/*
 * ESP32-S3 + ST7789 Display Demo
 * 硬件: Goouuu Tech ESP32-S3N15R8
 * 屏幕: ST7789 240x240 SPI TFT LCD
 * 库:   TFT_eSPI (by Bodmer)
 *
 * 接线:
 *   ST7789 →  ESP32-S3
 *   VCC    →  3.3V
 *   GND    →  GND
 *   CS     →  GPIO34  (FSPICS0)
 *   DC     →  GPIO7
 *   RST    →  GPIO6
 *   SCL    →  GPIO36  (FSPICLK)
 *   SDA    →  GPIO35  (FSPID)
 *   BL     →  GPIO21
 */

#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

static const uint16_t rainbow[] = {
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN,
  TFT_CYAN, TFT_BLUE, TFT_MAGENTA
};
static const int rainbowLen = sizeof(rainbow) / sizeof(rainbow[0]);

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nStarting...");

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  delay(10);
  digitalWrite(TFT_RST, HIGH);
  delay(120);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_RED);
  delay(1000);
  tft.fillScreen(TFT_GREEN);
  delay(1000);
  tft.fillScreen(TFT_BLUE);
  delay(1000);
  tft.fillScreen(TFT_BLACK);
  delay(500);

  // Demo
  demoStartup();
  delay(1500);
  demoShapes();
  delay(1500);
  demoText();
  delay(1500);
  demoRainbowCircles();
  delay(1500);
  demoBouncingBall();
}

void loop() {
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 3000) {
    lastUpdate = millis();
    showSystemInfo();
  }

  static int dotAngle = 0;
  dotAngle = (dotAngle + 4) % 360;
  tft.drawPixel(
    120 + 80 * cos(dotAngle * DEG_TO_RAD),
    120 + 80 * sin(dotAngle * DEG_TO_RAD),
    rainbow[(dotAngle / 10) % rainbowLen]
  );
  delay(10);
}

void demoStartup() {
  for (int i = 0; i < 120; i += 4) {
    tft.drawRect(120 - i, 120 - i, i * 2, i * 2, rainbow[(i / 4) % rainbowLen]);
    delay(5);
  }
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("ESP32-S3N15R8", 120, 90, 4);
  tft.drawCentreString("ST7789 Display", 120, 155, 2);
}

void demoShapes() {
  tft.fillRect(10, 10, 100, 55, TFT_RED);
  tft.fillRect(130, 10, 100, 55, TFT_GREEN);
  tft.fillRect(10, 85, 100, 55, TFT_BLUE);
  tft.fillRect(130, 85, 100, 55, TFT_YELLOW);
  tft.fillCircle(60, 200, 25, TFT_MAGENTA);
  tft.fillCircle(180, 200, 25, TFT_CYAN);
  tft.fillTriangle(120, 10, 150, 65, 90, 65, TFT_ORANGE);
}

void demoText() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Hello World!", 20, 20, 4);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("ESP32-S3", 20, 58, 4);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("240x240", 20, 96, 4);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("ST7789", 20, 134, 4);
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.drawString("16-bit Color", 20, 172, 2);
}

void demoRainbowCircles() {
  for (int r = 100; r > 0; r -= 5) {
    tft.drawCircle(120, 120, r, rainbow[(r / 5) % rainbowLen]);
    delay(10);
  }
  for (int r = 100; r > 0; r -= 5) {
    tft.fillCircle(120, 120, r, rainbow[(r / 5) % rainbowLen]);
    delay(15);
  }
}

void demoBouncingBall() {
  int x = 120, y = 60, dx = 3, dy = 2;
  for (int i = 0; i < 120; i++) {
    tft.fillCircle(x, y, 8, TFT_BLACK);
    x += dx;
    y += dy;
    if (x - 8 <= 0 || x + 8 >= 240) dx = -dx;
    if (y - 8 <= 0 || y + 8 >= 240) dy = -dy;
    tft.fillCircle(x, y, 8, rainbow[(x / 10 + y / 10) % rainbowLen]);
    delay(15);
  }
}

void showSystemInfo() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("System Info", 20, 15, 2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Chip: " + String(ESP.getChipModel()), 20, 45, 2);
  tft.drawString("Rev:  v" + String(ESP.getChipRevision()), 20, 68, 2);
  tft.drawString("Cores: " + String(ESP.getChipCores()), 20, 91, 2);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Free Heap: " + String(ESP.getFreeHeap() / 1024) + " KB", 20, 125, 2);
  tft.drawString("PSRAM: " + String(ESP.getPsramSize() / 1024 / 1024) + " MB", 20, 148, 2);
  tft.drawString("Flash: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB", 20, 171, 2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("WiFi MAC: " + WiFi.macAddress(), 20, 205, 1);
}
