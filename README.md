# Battery 插件

在原有电池百分比显示基础上，新增**剩余时间显示**功能。

### 任务栏显示项目

插件提供两个独立的显示项目，可在"显示设置"中分别勾选：

| 项目名 | ID | 功能 |
|--------|----|------|
| **Battery** | `b5R30ITQ` | 电池电量百分比（数字/图标/图标+数字） |
| **Battery time** | `b5R31TME` | 电池剩余使用/充电时间 |

### 时间显示格式

| 状态 | 格式 | 示例 |
|------|------|------|
| 放电中 | `Xh Xm` | `2h 31m` |
| 充电中（可估算） | `+Xh Xm` | `+1h 15m` |
| 充电中（不可估算） | 自定义（默认 `~`） | `~` |
| 已充满 | `Full` | `Full` |
| 电量极低 | `5m` | `5m` |

![任务栏效果](images/battery_time_screenshot.png)

### 安装

1. 将 `Battery.dll` 放入 TrafficMonitor 目录下的 `plugins` 文件夹
2. 重启 TrafficMonitor
3. 右键任务栏窗口 → **显示设置** → 勾选需要的项目

### 自定义设置

右键任务栏窗口 → **显示设置** → 找到 **Battery time** → 点击"插件选项"，可自定义：

| 选项 | 说明 |
|------|------|
| **电池显示方式** | 百分比：纯数字 / 纯图标 / 数字在图标旁 |
| **在鼠标提示显示电池信息** | 悬浮显示电量、充电状态、剩余时间 |
| **显示百分号** | 数字后是否加 `%` |
| **显示充电动画** | 充电时电池图标闪烁 |
| **Unknown:** | 充电无法估算时间时显示的文本（默认 `~`，可改成 `+...`、`N/A` 等） |

### 技术特点

- 使用 IOCTL_BATTERY_QUERY_STATUS 直接读取电池控制器原始数据（mWh/mW），精度高于 GetSystemPowerStatus
- 多电池自动聚合（双电池笔记本）
- EMA 平滑滤波（α=0.3），防止瞬时功率跳变导致显示抖动
- IOCTL 失败时自动回退到 GetSystemPowerStatus

### 构建

```powershell
msbuild TrafficMonitorPlugins.sln /p:Configuration=Release /p:Platform=x64
```

需要 Visual Studio 2022 + Windows 10 SDK + MFC 库。
