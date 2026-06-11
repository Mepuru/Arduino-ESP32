// tft_setup.h — 适用于 Goouuu Tech ESP32-S3N15R8 + ST7789
//
// 此文件放在 .ino 同目录下，TFT_eSPI 编译时会自动识别并加载。
// 关键: ESP32-S3 必须用 USE_HSPI_PORT，否则和 Flash 冲突导致崩溃。
// ============================================================

#define USER_SETUP_INFO "ESP32-S3N15R8 + ST7789"

// ---------- 显示屏驱动 ----------
#define ST7789_DRIVER

// ---------- 屏幕分辨率 ----------
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// ---------- ESP32-S3 关键配置!!! ----------
// 必须用 HSPI (SPI3_独立控制器)，不能用 FSPI (和 Flash 共用总线)
#define USE_HSPI_PORT

// ---------- 引脚定义 ----------
// HSPI 模式下引脚通过 GPIO Matrix 任意映射，选择安全引脚即可
#define TFT_CS   10      // Chip Select
#define TFT_DC   5       // Data/Command
#define TFT_RST  6       // Reset
#define TFT_MOSI 11      // MOSI / SDA
#define TFT_SCLK 12      // Clock / SCL
#define TFT_MISO -1      // MISO 未使用

// ---------- 背光 ----------
#define TFT_BL   21

// ---------- SPI 速率 ----------
#define SPI_FREQUENCY         40000000
#define SPI_READ_FREQUENCY    20000000

// ---------- 颜色顺序 ----------
#define TFT_RGB_ORDER TFT_RGB

// ---------- 字体 ----------
#define LOAD_GLCD    // Font 1 (8px, 默认)
#define LOAD_FONT2   // Font 2 (16px)
#define LOAD_FONT4   // Font 4 (26px)
#define LOAD_FONT6   // Font 6 (48px 数字)
#define LOAD_FONT7   // Font 7 (48px 七段数码管)
#define LOAD_FONT8   // Font 8 (75px 数字)
#define LOAD_GFXFF   // Adafruit FreeFonts
#define SMOOTH_FONT  // 平滑字体渲染
