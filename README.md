# Arduino ESP32 Projects

Goouuu Tech ESP32-S3N16R8 开发板系列项目。

---

## 项目列表

### [🔵 01_st7789_demo](01_st7789_demo/) — ST7789 显示屏驱动

ESP32-S3 + ST7789 240×240 SPI TFT LCD 的 Arduino 示例。

| | |
|---|---|
| **核心** | TFT_eSPI 库驱动 |
| **接线** | CS=10, DC=5, RST=6, MOSI=11, SCLK=12, BL=21 |
| **要点** | ESP32-S3 需 `USE_HSPI_PORT` 避免崩溃 |
| **演示** | 纯色切换、图形、文字、系统信息、弹球动画 |

---

### [🔵 ble_lock](02_ble_lock/) — BLE 门锁 Web 控制器

ESP32-S3 + ST7789，通过手机浏览器控制 BLE 门锁开锁，无需物理按钮。

| | |
|---|---|
| **核心** | Web Server + BLE + TFT_eSPI |
| **接线** | CS=10, DC=5, RST=6, MOSI=11, SCLK=12, BL=21 |
| **要点** | 协议已实现，配置 tokenId 即可使用 |
| **演示** | ST7789 状态屏、Web 控制页面、BLE 开锁 |

---
