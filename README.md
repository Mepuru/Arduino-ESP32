# ESP32-S3 + ST7789 Display Demo

Goouuu Tech ESP32-S3N15R8 核心板驱动 ST7789 TFT 显示屏的 Arduino 示例项目。

## 硬件

| 组件 | 型号 | 说明 |
|------|------|------|
| 主控 | Goouuu Tech **ESP32-S3N15R8** | Xtensa LX7 双核, 16MB Flash, 8MB PSRAM |
| 屏幕 | **ST7789** SPI TFT LCD | 240×240 / 240×320, RGB565 16-bit 色彩 |

## Arduino IDE 配置

### 1. 安装 ESP32 开发板支持

**Arduino IDE → 文件 → 首选项 → 附加开发板管理器网址**，添加：

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

然后 **工具 → 开发板 → 开发板管理器**，搜索 `ESP32` 并安装 **esp32** (建议 ≥2.0.14)。

### 2. 选择开发板

| 设置 | 值 |
|------|-----|
| 开发板 | **ESP32S3 Dev Module** |
| Flash Size | **16MB (128Mb)** |
| Partition Scheme | **Default 16MB with spiffs (3MB APP/1.5MB SPIFFS)** |
| PSRAM | **OPI PSRAM** |
| USB CDC On Boot | **Enabled** (原生 USB) / **Disabled** (外置串口芯片) |
| Upload Speed | **921600** |

> 💡 判断方式：插上 USB 后，如果设备管理器出现 COM 端口（如 CH343/CP2102），则是外置串口芯片，USB CDC 设 Disabled；如果出现 ESP32 开头的串口设备，则是原生 USB，设 Enabled。

### 3. 安装 TFT_eSPI 库

**工具 → 管理库** → 搜索 `TFT_eSPI` → 安装 **TFT_eSPI by Bodmer**

### 4. 完成 — 无需额外配置

项目里已经自带了 [`tft_setup.h`](st7789_demo/tft_setup.h)，放在 `.ino` 同目录下。
TFT_eSPI 编译时会自动检测并加载它，**完全不需要动全局库文件**。

> 💡 原理：`TFT_eSPI.h` 第 59 行用 `__has_include(<tft_setup.h>)` 检测草图目录下的配置文件，优先级高于全局 `User_Setup.h`。

## 接线图

```
ST7789      ESP32-S3
──────      ────────
VCC    ──→  3.3V
GND    ──→  GND
CS     ──→  GPIO10
DC     ──→  GPIO9
RST    ──→  GPIO8
SCL    ──→  GPIO12  (SPI Clock)
SDA    ──→  GPIO11  (SPI MOSI)
BL     ──→  GPIO45  (背光)
```

## 项目结构

```
Arduino-ESP32-Demo/
├── .gitignore
├── README.md
└── st7789_demo/            ← Arduino 草稿文件夹
    ├── st7789_demo.ino     ← 主程序
    └── tft_setup.h         ← TFT_eSPI 引脚配置（项目本地，无需全局）
```

## 演示内容

上电后依次播放：

1. **启动动画** — 从中心扩散的彩色方框 + 标题
2. **基本图形** — 矩形、圆、三角形
3. **文字渲染** — 不同字号、颜色、自动换行
4. **彩虹圆环** — 从外向内渐变的彩虹圈
5. **弹球动画** — 物理碰撞反弹
6. **系统信息** — 芯片型号、内存、Flash、WiFi MAC
7. **Loop 动画** — 旋转像素点 + 周期刷新系统信息

## 问题排查

| 现象 | 原因 | 解决 |
|------|------|------|
| 白屏/无显示 | 背光未开 | 检查 BL 引脚，或确认 `TFT_BL` 定义 |
| 显示错乱 | 引脚配置不对 | 核对 `User_Setup.h` 中的 GPIO 定义 |
| 颜色不对 | 屏幕初始化时序/旋转 | 尝试 `tft.setRotation(0..3)` |
| 编译报错 "tft_setup.h" 未找到 | `tft_setup.h` 不在草图目录 | 确认它与 `.ino` 在同一文件夹 |
| 上传失败 | 启动模式/端口错误 | 按住 BOOT 键再点上传，或检查端口设置 |
