# Blinker 远程控制开关演示

ESP32-S3 + ST7789，通过 Blinker App 远程控制开关状态。
App 中配置 4 个按钮，屏幕有多种显示模式响应。

## 硬件接线

| ST7789  | ESP32-S3 |
|---------|----------|
| VCC     | 3.3V     |
| GND     | GND      |
| CS      | GPIO10   |
| DC      | GPIO5    |
| RST     | GPIO6    |
| SCL     | GPIO12   |
| SDA     | GPIO11   |
| BL      | GPIO21   |

## Arduino IDE 配置

| 参数 | 值 |
|------|-----|
| Board | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Default 16MB with spiffs |
| PSRAM | OPI PSRAM |
| USB CDC On Boot | Disabled |
| Upload Speed | 921600 |

## 使用步骤

### 1. 安装 Blinker 库

将 Blinker SDK 文件夹放入 Arduino 的 `libraries/` 目录：

```
C:\Users\<你的用户名>\Documents\Arduino\libraries\blinker-library\
```

### 2. 配置 WiFi 和 Blinker Auth

复制 `config.example.h` 为 `config.h`，填入信息：

```cpp
#define WIFI_SSID   "你的WiFi名称"
#define WIFI_PASS   "你的WiFi密码"
#define BLINKER_AUTH "你的Blinker设备Secret Key"
```

### 3. 获取 Blinken Secret Key

1. 手机安装 **Blinker App**
2. App 中点击右上角 **+** → **独立设备** → **网络接入**
3. 选择 **阿里云**，复制生成的 **Secret Key**
4. 填入 `config.h` 的 `BLINKER_AUTH`

### 4. 编译上传

选择端口 → 编译上传。屏幕显示连接状态。

### 5. 配置 App 界面

1. 在 Blinker App 中打开设备
2. 点底部 **编辑界面**（铅笔图标）
3. 拖 4 个 **按键（Button）** 到界面
4. 分别设置键名：`btn-power`、`btn-color`、`btn-demo`、`btn-reset`
5. 保存退出

### 6. 演示效果

| 按钮 | 键名 | 屏幕反应 |
|------|------|---------|
| 开关 | `btn-power` | ON/OFF 大字切换，底部状态条联动 |
| 颜色 | `btn-color` | 主题色循环：绿→青→黄→紫→橙→白 |
| 模式 | `btn-demo` | 状态屏 → 图形屏 → 弹球屏 循环 |
| 重置 | `btn-reset` | 归零、关闭、恢复默认 |

### 3 种显示模式

- **状态屏**：IP、WiFi 信号、连接状态、ON/OFF 大字、切换次数
- **图形屏**：圆形、矩形、三角形、圆角矩形、椭圆
- **弹球屏**：弹球在屏幕内反弹，球上显示当前切换次数

## 关于米家 MIOT

项目包含 `BLINKER_MIOT_LIGHT` 模式尝试接入米家，但 **Blinker 官方已于 2025.7.7 停止 MIOT 语音助手服务**（因小爱等平台要求 3C 认证），实测米家 App 中无法搜索到设备。代码中保留 MIOT 回调，但已确认不可用。建议仅使用 Blinker App 控制。

## 注意事项

- `config.h` 不要提交到 Git（已在 .gitignore 中忽略）
- ESP32-S3 必须使用 `USE_HSPI_PORT`，否则 FSPI 与 Flash 冲突导致崩溃
- App 编辑界面时，按键的 **键名** 必须与代码中 `BlinkerButton` 的参数一致

## 问题排查

| 问题 | 排查 |
|------|------|
| 屏幕不亮 | 检查 BL 引脚是否 3.3V，CS/DC/RST 接线是否正确 |
| WiFi 连接失败 | 检查 Wi-Fi 名称密码，路由器 2.4G |
| Blinker 连接失败 | 检查 Secret Key 是否正确，设备是否在 App 中添加 |
| App 按钮没反应 | 检查键名是否一致，确认设备在线 |
| 屏幕显示异常/崩溃 | 确认 `tft_setup.h` 中 `USE_HSPI_PORT` 已定义 |
