# BLE Door Lock — ESP32-S3 无按钮 Web 控制器

通过手机浏览器控制 BLE 门锁开锁，无需物理按钮、无需专用 App。
ESP32-S3 自动完成 WiFi 联网 → API 获取凭据 → BLE 握手 → 开锁全流程。

## 硬件需求

| 组件 | 型号 |
|------|------|
| 主控 | Goouuu Tech ESP32-S3N16R8 (16MB Flash, 8MB PSRAM) |
| 显示屏 | ST7789 240×240 SPI TFT LCD (可选) |
| 门锁 | BLE 智能门锁 |

### ST7789 接线

| ST7789 | ESP32-S3 |
|--------|----------|
| VCC | 3.3V |
| GND | GND |
| CS | GPIO10 |
| DC | GPIO5 |
| RST | GPIO6 |
| SCL | GPIO12 |
| SDA | GPIO11 |
| BL | GPIO21 |

## Arduino IDE 配置

### 开发板设置

| 参数 | 值 |
|------|-----|
| Board | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Huge APP (3MB No OTA/SPIFFS) |
| PSRAM | OPI PSRAM |
| USB CDC On Boot | Disabled |
| Upload Speed | 921600 |

### 所需库

| 库 | 用途 | 安装 |
|----|------|------|
| **TFT_eSPI** by Bodmer | ST7789 驱动 | 工具 → 管理库 → 搜索安装 |
| **ArduinoJson** by Benoit Blanchon (≥ 6.x) | JSON 解析 | 工具 → 管理库 → 搜索安装 |
| ESP32 BLE | BLE 通信 | Arduino ESP32 核心自带 |
| mbedtls | AES-128-ECB 加解密 | Arduino ESP32 核心自带 |

## 快速开始

### 1. 配置

```bash
cp config.example.h config.h
```

编辑 `config.h`:

```cpp
#define WIFI_SSID       "你的WiFi名称"
#define WIFI_PASS       "你的WiFi密码"
#define LOCK_TOKEN      "抓包获取的tokenId"
#define API_HOST        "API服务器地址"
#define API_PORT        10015
#define API_METHOD_LIST "门锁列表API方法名"
#define API_METHOD_AUTH "门锁认证API方法名"
#define API_APP_ID      "App标识符"
#define BLE_DEFAULT_KEY "初始BLE密钥(16字节)"
```

> ⚠️ `config.h` 已加入 `.gitignore`，不会上传到仓库。
> 每个人只需维护自己的 `config.h`，参考 `config.example.h` 的格式。

### 2. 编译上传

Arduino IDE 打开 `02_ble_lock/` 文件夹（或 `02_ble_lock.ino`），选择正确端口后上传。

### 3. 使用

1. 上电后 TFT 屏幕显示 IP 地址（如 `192.168.1.100`）
2. 同一 WiFi 下手机浏览器访问 `http://192.168.1.100`
3. 点击「开锁」按钮，等待 ~20-30 秒

> 网页和屏幕实时显示进度：扫描 → 连接 → 握手 1/6 → 5/6 → 6/6 → 开锁成功

## 工作原理

### 启动流程

```
ESP32 上电
  │
  ├─ WiFi 连接
  ├─ 读取 NVS 中的 tokenId (如有)
  ├─ NTP 时间同步
  ├─ HTTPS → API 获取门锁列表 + BLE 凭据
  └─ 启动 Web 服务器, 等待指令
```

### 开锁流程 (六步握手)

```
手机浏览器点「开锁」
  │
  ├─ BLE 扫描 (最多 3 轮 × 8 秒)
  ├─ BLE 连接 + 服务发现
  ├─ 1/6 GET_SESSION_ID   → 获取会话 ID
  ├─ 2/6 GET_SECRET       → 获取动态 AES 密钥
  ├─ 3/6 GET_AUTH         → 发送 authCode 认证
  ├─ 4/6 GET_DNA_INFO     → 获取锁的协议版本
  ├─ 5/6 SET_SYSTEM_TIME  → 同步锁系统时间
  └─ 6/6 OPEN_LOCK        → 发送开锁命令 (v21 / v12 自适应)
  │
  └─ 完成 → 5 秒后自动回锁
```

### 安全

| 层 | 说明 |
|----|------|
| HTTPS | ESP32 通过 WiFi 调用 API 获取凭据，`setInsecure()` 跳过证书验证 |
| AES-128-ECB | BLE 帧加密，密钥由 API 下发 + 动态协商 |
| CRC16 | BLE 帧完整性校验 |
| Token 存储 | tokenId 存储在 ESP32 NVS 中，不掉失 |

## 功能特性

| 功能 | 说明 |
|------|------|
| **无按钮操作** | 全部通过手机浏览器完成 |
| **实时进度显示** | ST7789 屏幕 + Web 页面同步显示每一步 |
| **帧缓冲渲染** | TFT_eSPI Sprite 帧缓冲，屏幕无闪烁 |
| **Token 在线更新** | 访问 `/settings` 粘贴新 token，无需重新编译 |
| **多扫描轮次** | 最高 3 轮 × 8 秒扫描，适应长广播间隔的锁 |
| **协议自适应** | 自动尝试 v21 + authStartTime，v12 降级 |
| **自动回锁** | 开锁后 3 秒倒计时 → 5 秒复位到锁定状态 |
| **WiFi/BLE 共存** | BLE 只初始化一次，不反复开关，防崩溃 |
| **响应式 Web 页** | 手机/PC 浏览器自适应布局 |
| **配置文件 gitignored** | `config.h` 不提交，敏感信息安全 |

## 项目结构

```
02_ble_lock/
├── 02_ble_lock.ino         # 主程序 (WiFi + WebServer + TFT + 调度)
├── config.h                 # 你的实际配置 (gitignored)
├── config.example.h         # 配置模板 (上传仓库)
├── lock_service.h           # BLE 协议实现 (AES/CRC/六步握手)
├── web_ui.h                 # 嵌入式 Web 控制页面 (HTML/CSS/JS)
├── tft_setup.h              # TFT_eSPI 引脚/驱动配置
└── README.md
```

## 获取 tokenId

tokenId 通过抓包从微信小程序获取，有效期较长。

> 方式不限，只要能从微信小程序的 API 请求中提取到 `tokenId` 字段即可。
> 获取后通过 ESP32 的 Web 设置页面 `/settings` 在线更新，无需重新编译。

## 问题排查

| 问题 | 可能原因 | 解决 |
|------|----------|------|
| TFT 崩溃重启 | 未用 `USE_HSPI_PORT` | 确认 `tft_setup.h` 中有 `#define USE_HSPI_PORT` |
| WiFi 连不上 | SSID/密码错、距离远 | 检查 `config.h` |
| 扫描不到锁 | 锁不广播、手机连着锁 | 关手机蓝牙再试，或确认锁在 5m 内 |
| API 返回 304 | tokenId 过期 | 重新抓包 → 在 `/settings` 更新 |
| BLE 连接失败 | 信号弱、锁已连其他设备 | 确保锁空闲，靠近锁 |
| Web 打不开 | IP 错、不在同一网段 | 看 TFT 屏幕 IP 地址 |
| 开锁 status=6 | 命令格式不对 | 确认协议版本和参数与小程序一致 |
| ESP32 反复重启 | BLE init/deinit 循环 | 已修复: BLE 只初始一次 |

## 技术细节

### 帧结构

```
HSJ + totalLen(2) + flagProto(2) + AES-128-ECB 加密负载 + CRC16(2)
```

### 加密负载

```
sessionId(4) + snr(1) + rfu(1) + cmd(1) + status(1) + flag(1) + keyGroupId(2) + cmdVer(2) + iterable
```

### 协议版本

- 默认 `cmdVer = 12`（小程序 codec.js 默认值）
- 开锁时尝试 `cmdVer = 21` + `authStartTime`（协议版本 ≥ 21）
- 失败后自动降级到 `cmdVer = 12`，不含 `authStartTime`

## License

MIT
