// User_Setup.h — 适用于 Goouuu Tech ESP32-S3N15R8 + ST7789
//
// 使用说明:
//   安装 TFT_eSPI 库后，将此文件复制到 Arduino 的 TFT_eSPI 库目录下
//   覆盖原有的 User_Setup.h:
//     <Arduino>/libraries/TFT_eSPI/User_Setup.h
//
//   或者在 TFT_eSPI 库的 User_Setup_Select.h 中取消对应注释
//   并#include 此文件的路径。
//
//   若不想覆盖，可将备份的 User_Setup.h 重命名保留。
// ============================================================

#define USER_SETUP_INFO "ESP32-S3N15R8 + ST7789"

// ---------- 显示屏驱动 ----------
#define ST7789_DRIVER

// ---------- 屏幕分辨率 ----------
#define TFT_WIDTH  240
#define TFT_HEIGHT 240
// 如果是 240x320 的 ST7789 屏幕，改为:
// #define TFT_WIDTH  240
// #define TFT_HEIGHT 320

// ---------- 引脚定义 (ESP32-S3 GPIO) ----------
#define TFT_CS   10      // Chip Select
#define TFT_DC   9       // Data/Command
#define TFT_RST  8       // Reset
#define TFT_MOSI 11      // MOSI (SDA)
#define TFT_SCLK 12      // Clock (SCL)
#define TFT_MISO -1      // MISO 未使用

// ---------- 背光引脚 ----------
#define TFT_BL   45      // 背光控制 (如不需要 PWM 调光，可注释掉)

// ---------- SPI 速率 ----------
#define SPI_FREQUENCY         40000000   // 40MHz
#define SPI_READ_FREQUENCY    20000000   // 20MHz

// ---------- 旋转方向 (0/1/2/3) ----------
// 可在代码 tft.setRotation(n) 动态设置，此处不固定
// #define TFT_ROTATION 1

// ---------- 字体支持 ----------
#define SMOOTH_FONT
