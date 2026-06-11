# CLAUDE.md — Agent 开发规范

## 项目概述

Goouuu Tech **ESP32-S3N16R8** (16MB Flash, 8MB PSRAM) 开发板系列 Arduino 项目。

## 仓库结构

```
Arduino-ESP32/
├── CLAUDE.md              ← 本文件（Agent 规范）
├── README.md              ← 顶层项目列表（只列项目名和一句话简介）
├── .gitignore
└── <project_name>/        ← 每个项目一个独立文件夹
    ├── <project_name>.ino ← Arduino 主程序
    ├── tft_setup.h        ← TFT_eSPI 配置（如涉及屏幕）
    └── README.md          ← 项目详细文档（接线、配置、排错）
```

### 添加新项目

1. 新建文件夹 `02_xxx/`
2. 放入 `.ino` 及所需配置文件
3. 在顶层 `README.md` 项目列表加一行
4. 在项目文件夹内写详细的 `README.md`

## 硬件要点

| 项目 | 值 |
|------|-----|
| 主控 | Goouuu Tech **ESP32-S3N16R8** |
| Flash | 16MB |
| PSRAM | 8MB (OPI PSRAM) |
| 上传接口 | CH340 (COM口), USB CDC On Boot = Disabled |

## Arduino IDE 通用设置

| 参数 | 值 |
|------|-----|
| Board | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Default 16MB with spiffs |
| PSRAM | OPI PSRAM |
| USB CDC On Boot | Disabled |
| Upload Speed | 921600 |

## TFT_eSPI 注意事项

- ESP32-S3 必须用 `USE_HSPI_PORT`，否则 FSPI 与 Flash 冲突 → StoreProhibited 崩溃
- 配置文件用项目本地的 `tft_setup.h`，TFT_eSPI 通过 `__has_include(<tft_setup.h>)` 自动加载
- 不修改全局库文件
- 字体需显式加载：`LOAD_GLCD`, `LOAD_FONT2`, `LOAD_FONT4` 等

## 命名规范

- 文件夹名: `snake_case` (如 `st7789_demo`)
- `.ino` 文件名: 与文件夹同名
- 常量/宏: `UPPER_SNAKE_CASE`
- 函数/变量: `snake_case`

## Git 提交

- 提交信息用英文
- 前缀: `feat:`, `fix:`, `docs:`, `tweak:`, `refactor:`, `test:`
- 保持单次提交聚焦单一改动

## 文档规范

- 顶层 README 只列项目列表，不做详细说明
- 每个项目文件夹内必须有 `README.md`，包含：
  - 硬件接线图
  - Arduino IDE 配置步骤
  - 项目特定注意事项
  - 演示内容说明
  - 问题排查表
- 注释用英文或中文均可，保持一致性
