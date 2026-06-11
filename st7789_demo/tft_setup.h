// tft_setup.h — 适用于 Goouuu Tech ESP32-S3N15R8 + ST7789
//
// 此文件放在 .ino 同目录下，TFT_eSPI 库编译时会自动识别并加载，
// 无需修改全局库文件。
// 原理: TFT_eSPI.h 通过 __has_include(<tft_setup.h>) 自动检测。
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
#define TFT_BL   45      // 背光控制

// ---------- SPI 控制器 ----------
// 0 = FSPI (SPI2, 与 Flash 共用总线), 1 = HSPI (SPI3, 独立)
#define TFT_SPI_PORT 1

// ---------- SPI 速率 ----------
#define SPI_FREQUENCY         27000000   // 27MHz（降速提高稳定性）
#define SPI_READ_FREQUENCY    10000000   // 10MHz

// ---------- 颜色顺序 ----------
#define TFT_RGB_ORDER TFT_RGB

// ---------- 初始化序列 ----------
// 某些 ST7789 模组需要备选初始化序列
#define INIT_SEQUENCE_3

// ---------- 旋转方向 (0/1/2/3) ----------
// 可在代码 tft.setRotation(n) 动态设置

// ---------- 字体支持 ----------
#define SMOOTH_FONT
