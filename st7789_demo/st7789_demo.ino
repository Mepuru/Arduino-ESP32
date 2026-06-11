/*
 * ESP32-S3 + ST7789 显示屏演示
 * ================================
 * 硬件: Goouuu Tech ESP32-S3N15R8
 * 屏幕: ST7789 240x240 SPI TFT LCD
 * 库:   TFT_eSPI (by Bodmer)
 *
 * 接线 (ST7789 -> ESP32-S3):
 *   VCC  -> 3.3V
 *   GND  -> GND
 *   CS   -> GPIO10
 *   DC   -> GPIO9
 *   RST  -> GPIO8
 *   SCL  -> GPIO12
 *   SDA  -> GPIO11
 *   BL   -> GPIO45
 *
 * 安装 Arduino 库 (库管理器搜索安装):
 *   1. TFT_eSPI by Bodmer
 *   2. 将本仓库的 User_Setup.h 复制到
 *      libraries/TFT_eSPI/User_Setup.h (覆盖)
 */

#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// ------------- 颜色表 -------------
// 预定义的 TFT_eSPI 颜色: BLACK, NAVY, DARKGREEN, DARKCYAN,
// MAROON, PURPLE, OLIVE, LIGHTGREY, DARKGREY, BLUE, GREEN,
// CYAN, RED, MAGENTA, YELLOW, WHITE, ORANGE, GREENYELLOW, PINK

static const uint16_t rainbow[] = {
  TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_GREEN,
  TFT_CYAN, TFT_BLUE, TFT_MAGENTA
};
static const int rainbowLen = sizeof(rainbow) / sizeof(rainbow[0]);

// ======================== SETUP ========================

void setup() {
  Serial.begin(115200);
  delay(500);  // 等待串口就绪
  Serial.println("\nESP32-S3 + ST7789 Demo Starting...");

  // 开背光
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // 初始化屏幕
  tft.init();
  tft.setRotation(1);      // 0/1/2/3 四个方向，根据安装方向调整
  tft.fillScreen(TFT_BLACK);

  // --- 依次播放演示段落 ---
  demoStartup();           // 启动动画
  delay(1500);
  demoShapes();            // 图形展示
  delay(1500);
  demoText();              // 文字展示
  delay(1500);
  demoRainbowCircles();    // 彩虹圈动画
  delay(1500);
  demoBouncingBall();      // 弹球动画
  delay(500);
}

// ======================== LOOP ========================

void loop() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if (now - lastUpdate >= 3000) {
    lastUpdate = now;
    showSystemInfo();
  }

  // 在等待间隙做点小动画: 画一个旋转的颜色点
  static int dotAngle = 0;
  dotAngle = (dotAngle + 4) % 360;

  tft.drawPixel(
    120 + 80 * cos(dotAngle * DEG_TO_RAD),
    120 + 80 * sin(dotAngle * DEG_TO_RAD),
    rainbow[(dotAngle / 10) % rainbowLen]
  );
  delay(10);
}

// ======================== 演示段落 ========================

// -------- 1. 启动动画 ----------
void demoStartup() {
  // 从中心向外画方框
  tft.fillScreen(TFT_BLACK);
  for (int i = 0; i < 120; i += 4) {
    tft.drawRect(120 - i, 120 - i, i * 2, i * 2, rainbow[(i / 4) % rainbowLen]);
    delay(5);
  }
  delay(200);

  // 显示标题
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawCentreString("Goouuu Tech", 120, 90, 4);
  tft.drawCentreString("ESP32-S3N15R8", 120, 120, 4);
  tft.drawCentreString("ST7789 Display", 120, 155, 2);
}

// -------- 2. 基本图形 ----------
void demoShapes() {
  tft.fillScreen(TFT_BLACK);

  // 四个彩色矩形
  tft.fillRect(10, 10, 100, 55, TFT_RED);
  tft.fillRect(130, 10, 100, 55, TFT_GREEN);
  tft.fillRect(10, 85, 100, 55, TFT_BLUE);
  tft.fillRect(130, 85, 100, 55, TFT_YELLOW);

  // 圆
  tft.fillCircle(60, 200, 25, TFT_MAGENTA);
  tft.fillCircle(180, 200, 25, TFT_CYAN);
  tft.drawCircle(120, 200, 55, TFT_WHITE);

  // 三角形
  tft.fillTriangle(120, 10, 150, 65, 90, 65, TFT_ORANGE);
}

// -------- 3. 文字展示 ----------
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

  // 测量文字宽度示例
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  int w = tft.textWidth("Hello World!", 4);
  tft.drawString("Width=" + String(w) + "px", 20, 200, 2);
}

// -------- 4. 彩虹圆环动画 ----------
void demoRainbowCircles() {
  tft.fillScreen(TFT_BLACK);

  for (int r = 100; r > 0; r -= 5) {
    tft.drawCircle(120, 120, r, rainbow[(r / 5) % rainbowLen]);
    delay(10);
  }

  // 从外向内逐个填充
  for (int r = 100; r > 0; r -= 5) {
    tft.fillCircle(120, 120, r, rainbow[(r / 5) % rainbowLen]);
    delay(15);
  }
}

// -------- 5. 弹球动画 ----------
void demoBouncingBall() {
  int x = 120, y = 60, dx = 3, dy = 2;
  int rad = 8;

  tft.fillScreen(TFT_BLACK);

  for (int i = 0; i < 120; i++) {
    // 擦除旧位置
    tft.fillCircle(x, y, rad, TFT_BLACK);

    x += dx;
    y += dy;

    // 边界碰撞
    if (x - rad <= 0 || x + rad >= 240) dx = -dx;
    if (y - rad <= 0 || y + rad >= 240) dy = -dy;

    // 画新球，颜色随位置变化
    int ci = (x / 10 + y / 10) % rainbowLen;
    tft.fillCircle(x, y, rad, rainbow[ci]);

    delay(15);
  }
}

// -------- 6. 系统信息 ----------
void showSystemInfo() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("System Info", 20, 15, 2);

  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Chip: " + String(ESP.getChipModel()), 20, 45, 2);
  tft.drawString("Rev:  v" + String(ESP.getChipRevision()), 20, 68, 2);
  tft.drawString("Cores: " + String(ESP.getChipCores()), 20, 91, 2);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  unsigned long freeHeap = ESP.getFreeHeap();
  unsigned long psramSize = ESP.getPsramSize();
  unsigned long flashSize = ESP.getFlashChipSize() / (1024 * 1024);

  tft.drawString("Free Heap: " + String(freeHeap / 1024) + " KB", 20, 125, 2);
  tft.drawString("PSRAM: " + String(psramSize / 1024 / 1024) + " MB", 20, 148, 2);
  tft.drawString("Flash: " + String(flashSize) + " MB", 20, 171, 2);

  // Wi-Fi MAC 地址
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("WiFi MAC: " + WiFi.macAddress(), 20, 205, 1);
}
