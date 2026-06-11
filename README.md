# ESP32-S3 + ST7789 Display Demo

Goouuu Tech **ESP32-S3N15R8** 核心板驱动 **ST7789** 240×240 SPI TFT 显示屏的 Arduino 示例项目。

## 硬件

| 组件 | 型号 | 说明 |
|------|------|------|
| 主控 | Goouuu Tech **ESP32-S3N15R8** | Xtensa LX7 双核, 16MB Flash, 8MB PSRAM |
| 屏幕 | **ST7789** SPI TFT LCD | 240×240, RGB565 16-bit 色彩 |

## 接线

```
ST7789      ESP32-S3    GPIO
─────────────────────────────
VCC    ──→  3.3V
GND    ──→  GND
CS     ──→  10
DC     ──→  5
RST    ──→  6
SCL    ──→  12          (SPI Clock)
SDA    ──→  11          (SPI MOSI)
BL     ──→  21          (背光)
```

## Arduino IDE 配置

### 1. 安装 ESP32 开发板支持

**文件 → 首选项 → 附加开发板管理器网址**，添加：

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

然后 **工具 → 开发板 → 开发板管理器**，搜索 `ESP32` 并安装 **esp32** (≥2.0.14)。

### 2. 选择开发板

| 设置 | 值 |
|------|-----|
| 开发板 | **ESP32S3 Dev Module** |
| Flash Size | **16MB (128Mb)** |
| Partition Scheme | **Default 16MB with spiffs** |
| PSRAM | **OPI PSRAM** |
| USB CDC On Boot | **Disabled** (外置串口芯片 CH340) |
| Upload Speed | **921600** |

### 3. 安装 TFT_eSPI 库

**工具 → 管理库** → 搜索 `TFT_eSPI` → 安装 **TFT_eSPI by Bodmer**

### 4. 完成 — 无需额外配置

项目自带 [`tft_setup.h`](st7789_demo/tft_setup.h)，放在 `.ino` 同目录下，
TFT_eSPI 编译时自动加载它，**不修改任何全局库文件**。

## ⚠️ ESP32-S3 必知: USE_HSPI_PORT

ESP32-S3 上 TFT_eSPI 的 `tft.init()` 会崩溃 (StoreProhibited)，
因为 **FSPI (SPI2_HOST) 和内部 Flash 共用总线**。

解决方案是在配置中加上：

```cpp
#define USE_HSPI_PORT
```

这会强制使用 **HSPI (SPI3_HOST)** 独立控制器，彻底避免冲突。
本项目已自带此配置。

> 参考: [Bodmer/TFT_eSPI Issue #3568](https://github.com/Bodmer/TFT_eSPI/issues/3568)

## 项目结构

```
Arduino-ESP32-Demo/
├── .gitignore
├── README.md
└── st7789_demo/
    ├── st7789_demo.ino     ← 主程序
    └── tft_setup.h         ← TFT_eSPI 配置（项目本地）
```

## 演示内容

上电后依次播放：

1. **纯色切换** — 红 → 绿 → 蓝 → 黑
2. **基本图形** — 矩形、圆、三角形
3. **文字渲染** — 不同颜色、字号
4. **系统信息** — 芯片型号、内存、Flash、WiFi MAC
5. **弹球动画** — loop 中彩色弹球

## 问题排查

| 现象 | 原因 | 解决 |
|------|------|------|
| 编译/上传通过，屏幕只亮背光无内容 | 接线不对或引脚不匹配 | 核对接线和 `tft_setup.h` 中的 GPIO |
| `tft.init()` 崩溃（StoreProhibited） | ESP32-S3 上缺 `USE_HSPI_PORT` | 在 `tft_setup.h` 中加入此宏定义 |
| 上传失败卡在 `Connecting...` | 未进入下载模式 | 按住 BOOT 键再点上传 |
| 串口输出乱码 | 波特率不对或端口选错 | 选 COM 口, 设 115200 baud |
