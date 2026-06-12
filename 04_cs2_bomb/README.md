# C4 Bomb Simulator

ESP32-S3 + ST7789 + 蜂鸣器，Web 页面控制的炸弹下包/拆包模拟器。

两种拆弹方式：**密码拆包**或**剪线小游戏**。倒计时 45 秒，蜂鸣器随剩余时间变速报警。

---

## 接线

### ST7789

| ST7789 | ESP32-S3 |
|--------|----------|
| VCC    | 3.3V     |
| GND    | GND      |
| CS     | GPIO10   |
| DC     | GPIO5    |
| RST    | GPIO6    |
| SCL    | GPIO12   |
| SDA    | GPIO11   |
| BL     | GPIO21   |

### Buzzer (低电平触发)

| Buzzer | ESP32-S3 |
|--------|----------|
| VCC    | 3.3V     |
| GND    | GND      |
| I/O    | GPIO4    |

---

## Arduino IDE 配置

| 参数 | 值 |
|------|-----|
| Board | ESP32S3 Dev Module |
| Flash Size | 16MB (128Mb) |
| Partition Scheme | Default 16MB with spiffs |
| PSRAM | OPI PSRAM |
| USB CDC On Boot | Disabled |
| Upload Speed | 921600 |

---

## 配置

编辑 `config.h`：

```cpp
#define WIFI_SSID       "your_wifi"
#define WIFI_PASS       "your_password"

#define PLANT_PASSWORD   "7355608"   // 下包密码 (7位)
#define DEFUSE_PASSWORD  "1234567"   // 拆包密码 (7位)
```

---

## 使用方法

1. 编译上传，查看 ST7789 显示的 IP 地址
2. 手机/电脑浏览器访问该 IP

### 下包 (IDLE 状态)

- **数字键盘**：输入 7 位密码 `7355608` → OK 下包

### 拆弹 (ARMED 状态)

两种方式，通过 PIN / GAME 标签切换：

**方式 A — 密码**
- 数字键盘输入 7 位密码 `1234567`

**方式 B — 剪线游戏**
- 提示 "Cut the RED wire!"，文字颜色随机（Stroop 干扰）
- 从 6 个纯色块中点击对应颜色
- 提示 2.5 秒后消失，靠记忆找颜色
- 每轮时间递减：7s → 6s → 5s → 4s → 3s
- 剪错或超时 → 分数归零重来
- 累计 **7 分** → 拆除成功

---

## 蜂鸣器节奏

| 剩余时间 | 蜂鸣间隔 | 屏幕颜色 |
|----------|---------|---------|
| 45s - 30s | 每 5 秒 | 白色 |
| 30s - 15s | 每 2 秒 | 黄色 |
| 15s - 10s | 每 1 秒 | 橙色 |
| 10s - 5s | 每 0.5 秒 | 红色 |
| 5s - 0s | 每 0.2 秒 | 红色 |
| 0s (爆炸) | 连续 3 秒 | 红色, EXPLODED |
| 剪线正确 | 短响 80ms | 显示正确提示 |
| 剪线错误/超时 | 长响 500ms | 显示错误提示 |
| 拆除成功 | 静音 | 绿色, DEFUSED |

---

## Web 页面功能

- **语言切换**：EN / 中文，右上角切换
- **大号倒计时**：实时显示，本地插值消除网络延迟
- **进度条**：颜色随剩余时间从白→黄→橙→红渐变
- **数字密码盘**：7 位 PIN 输入，输错抖动反馈
- **剪线游戏**：6 色块、Stroop 文字颜色干扰、2.5s 记忆挑战
- **ST7789 同步**：显示状态、倒计时、进度条、游戏分数

---

## API 接口

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/` | Web 控制页面 |
| GET | `/api/status` | 状态 JSON |
| POST | `/api/plant` | 下包 (body: password=xxx) |
| POST | `/api/defuse` | 密码拆包 (body: password=xxx) |
| POST | `/api/reset` | 重置到 IDLE |
| POST | `/api/game/start` | 开始剪线游戏 |
| POST | `/api/game/cut` | 剪线 (body: color=xxx) |
| POST | `/api/game/abort` | 退出游戏回 PIN 模式 |

---

## 问题排查

| 现象 | 原因 | 解决 |
|------|------|------|
| TFT 白屏/崩溃 | FSPI 与 Flash 冲突 | 确认 tft_setup.h 有 `USE_HSPI_PORT` |
| 蜂鸣器不响 | 接线/电压 | 确认 VCC 接 3.3V，I/O 接 GPIO4 |
| 蜂鸣器一直响 | 电平错误 | 确认是**低电平触发**型，HIGH=静音 |
| WiFi 连不上 | 密码错误 | 检查 config.h 中的 WiFi 凭据 |
| 页面打不开 | IP 不对 | 查看 TFT 上显示的 IP 地址 |
| 网页时间不准 | 网络延迟 | 已用本地插值补偿，误差 <0.5s |
