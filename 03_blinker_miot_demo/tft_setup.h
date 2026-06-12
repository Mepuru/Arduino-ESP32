#define USER_SETUP_INFO "ESP32-S3N16R8 + ST7789"

#define ST7789_DRIVER
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

#define USE_HSPI_PORT

#define TFT_CS   10
#define TFT_DC   5
#define TFT_RST  6
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_MISO -1

#define TFT_BL   21

#define SPI_FREQUENCY         40000000
#define SPI_READ_FREQUENCY    20000000

#define TFT_RGB_ORDER TFT_RGB

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT
