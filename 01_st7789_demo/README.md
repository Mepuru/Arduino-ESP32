# st7789_demo — ST7789 显示屏驱动

Goouuu Tech **ESP32-S3N16R8** (16MB Flash, 8MB PSRAM) 驱动 **ST7789 240×240 SPI TFT LCD** 的 Arduino 示例。

## 硬件接线

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

### 1. 安装 ESP32 核心

**文件 → 首选项 → 附加开发板管理器网址**，添加：

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

**工具 → 开发板 → 开发板管理器** → 搜索 `ESP32` → 安装 **esp32** (≥ 2.0.14)。

### 2. 板型设置

| 参数 | 值 |
|------|-----|
| Board | **ESP32S3 Dev Module** |
| Flash Size | **16MB (128Mb)** |
| Partition Scheme | **Default 16MB with spiffs** |
| PSRAM | **OPI PSRAM** |
| USB CDC On Boot | **Disabled** (通过 CH340 串口) |
| Upload Speed | **921600** |

### 3. 安装 TFT_eSPI 库

**工具 → 管理库** → 搜索 `TFT_eSPI` → 安装 **TFT_eSPI by Bodmer** (≥ 2.5.0)。

### 4. 打开项目

Arduino IDE → **文件 → 打开** → 选择 `st7789_demo.ino`。

项目自带的 [`tft_setup.h`](tft_setup.h) 会被自动加载，无需修改全局库。

## ⚠️ ESP32-S3 关键修复

本项目通过 `#define USE_HSPI_PORT` 解决 ESP32-S3 上的 SPI 崩溃问题：

```
原因: ESP32-S3 的 FSPI (SPI2_HOST) 与内部 Flash 共用总线
      TFT_eSPI 初始化时发生冲突 → StoreProhibited 崩溃

解决: 改用 HSPI (SPI3_HOST) 独立控制器
      在 tft_setup.h 中定义 USE_HSPI_PORT
      引脚通过 GPIO Matrix 路由，无需改接线
```

> 参考: [Bodmer/TFT_eSPI Issue #3568](https://github.com/Bodmer/TFT_eSPI/issues/3568)

## 引脚配置 (`tft_setup.h`)

| 定义 | 值 | 说明 |
|------|-----|------|
| `ST7789_DRIVER` | — | 选择 ST7789 驱动 |
| `TFT_WIDTH` | 240 | 屏幕宽度 |
| `TFT_HEIGHT` | 240 | 屏幕高度 |
| `USE_HSPI_PORT` | — | 强制使用 HSPI 控制器 |
| `TFT_CS` | 10 | 片选 |
| `TFT_DC` | 5 | 数据/命令 |
| `TFT_RST` | 6 | 复位 |
| `TFT_MOSI` | 11 | SPI 数据 |
| `TFT_SCLK` | 12 | SPI 时钟 |
| `TFT_BL` | 21 | 背光 |
| `SPI_FREQUENCY` | 40000000 | SPI 速率 40MHz |

## 演示内容

上电后自动播放：

| 顺序 | 内容 | 时长 |
|------|------|------|
| 1 | 🔴🟢🔵 纯色切换 | 各 1.5s |
| 2 | 📐 彩色图形（矩形、圆、三角形） | 3s |
| 3 | 🔤 文字展示（不同字号、颜色） | 3s |
| 4 | ℹ️ 系统信息（芯片、内存、Flash、MAC） | 5s |
| 5 | 🏀 弹球动画（loop 持续运行） | 持续 |

## 问题排查

| 现象 | 原因 | 解决 |
|------|------|------|
| 编译就崩溃 / StoreProhibited | 缺 `USE_HSPI_PORT` | 检查 `tft_setup.h` 有此宏定义 |
| 背光亮、屏幕无内容 | 接线不对或引脚不匹配 | 核对接线图和 `tft_setup.h` 的 GPIO |
| 上传卡 `Connecting...` | 未进入下载模式 | 按住 BOOT 键再点上传 |
| 文字全空白 | 字体加载宏缺失 | 确认 `tft_setup.h` 中有 `LOAD_FONT2` 等 |
| 串口乱码 | 端口/波特率不对 | 选 COM 口, 设 115200 baud |

## 文件结构

```
st7789_demo/
├── st7789_demo.ino     ← Arduino 主程序
├── tft_setup.h         ← TFT_eSPI 配置（引脚、字体、SPI 端口）
└── README.md           ← 本文件
```
