# TrafficMonitorPlugins

本仓库是 [TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 的插件集合。
基于上游 [zhongyang219/TrafficMonitorPlugins](https://github.com/zhongyang219/TrafficMonitorPlugins)，
可随时拉取更新。

## 包含的插件

| 插件 | 目录 | 说明 |
|------|------|------|
| Battery | `Plugins/Battery/` | 电池电量 + 剩余时间显示 |
| DateTime | `Plugins/DateTime/` | 日期时间显示 |
| Weather | `Plugins/Weather/` | 天气显示 |
| 其他 | `Plugins/*` | 见上游仓库 |

---

## Battery 插件

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
| 放电中 | `Xh Xm` | `7h 31m` |
| 充电中 | `+Xh Xm` | `+1h 30m` |
| 已充满 | `Full` | `Full` |

![任务栏效果](images/battery_time_screenshot.png)

### 安装

1. 将 `Battery.dll` 放入 TrafficMonitor 目录下的 `plugins` 文件夹
2. 重启 TrafficMonitor
3. 右键任务栏窗口 → **显示设置** → 勾选需要的项目

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

---

*其余插件说明请参见上游仓库的 [插件下载页](./download/plugin_download.md)。*
