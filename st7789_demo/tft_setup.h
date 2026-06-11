// tft_setup.h — 适用于 Goouuu Tech ESP32-S3N15R8 + ST7789
//
// 此文件放在 .ino 同目录下，TFT_eSPI 库编译时会自动识别并加载
// 无需修改全局库文件。
// ============================================================

#define USER_SETUP_INFO "ESP32-S3N15R8 + ST7789"

// ---------- 显示屏驱动 ----------
#define ST7789_DRIVER

// ---------- 屏幕分辨率 ----------
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// ---------- 引脚定义 ----------
// 改用安全引脚 (避开 GPIO 8-12, 这些被内部 Flash 占用)
#define TFT_CS   4       // Chip Select
#define TFT_DC   5       // Data/Command
#define TFT_RST  6       // Reset
#define TFT_MOSI 7       // MOSI (SDA)
#define TFT_SCLK 15      // Clock (SCL)
#define TFT_MISO -1      // MISO 未使用

// ---------- 背光 ----------
#define TFT_BL   16      // 背光控制

// ---------- SPI 速率 ----------
#define SPI_FREQUENCY         40000000
#define SPI_READ_FREQUENCY    20000000

// ---------- 颜色顺序 ----------
#define TFT_RGB_ORDER TFT_RGB

// ---------- 字体 ----------
#define SMOOTH_FONT
