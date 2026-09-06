# AILimit - TrafficMonitor AI 额度插件

在任务栏显示 AI 额度/余额：OpenCode Go（5h/周/月多窗口）、OpenRouter 余额、DeepSeek 余额+今日消耗，支持用完/过期自动隐藏。

![任务栏效果](images/taskbar.png)
![设置界面](images/options.png)

## 功能

- Go 多窗口剩余额度（5 小时 / 周 / 月），重置时间显示
- OpenRouter 余额（上排）+ 每日消耗（下排）
- DeepSeek 总余额（上排）+ 今日消耗（下排，本地差值统计）
- 用完 / 过期 / 密钥失效自动隐藏（宽归零塌陷，不改任务栏布局）
- 密钥 DPAPI 加密存本地 ini，明文自动迁移清理
- 异步测试按钮，中文内联结果；预设（推荐/极简/详细/自定义）+ 真实数据预览
- 深色 / 浅色模式自动适配

## 安装

1. 从 [Release](https://github.com/Mariomoprc/TrafficMonitorPlugins/releases/tag/AILimit_V1.2.3) 下载 `AILimitPlugin.dll`
2. 放到 TrafficMonitor 所在目录的 `plugins` 文件夹下（没有就创建）
3. 重启 TrafficMonitor
4. 右键任务栏 → **显示设置** → 勾选 `Go Top/Bottom`、`OR Top/Bottom`、`DS Top/Bottom`
5. 右键任务栏 → **显示设置** → 选中任意 AILimit 项 → **插件选项** → 填密钥 → 点右侧测试

## 设置说明

| 分组 | 说明 |
|------|------|
| 预设 | 推荐（Go上+下+OR上+DS上）/ 极简（仅Go上）/ 详细（全部）/ 自定义（按勾选反推） |
| API 密钥 | Go×2、OR、DS，密码框可显隐，测试异步不卡界面 |
| 端点 | Go 端点必填（自填额度接口地址）；OR/DS 默认官方接口，一般不用动 |
| 显示项 | 8 项独立开关；效果（进度条/简约文字/环形）、颜色、宽度、标签 |
| 其他 | 重置时间、刷新秒、用完过期自动隐藏、电池省电降帧 |
| 预览 | 真实数据；无数据标示例；已隐藏项标 [藏] |

今日消耗（DS 下排）为本地差值：`今日首次总额 − 当前总额`，跨天重建基准；充值到账自动归零并在悬浮提示标注“今日有入账”。

## 构建

需要 Visual Studio 2022 + Windows 10 SDK（无需 MFC）。

```powershell
msbuild TrafficMonitorPlugins.sln /p:Configuration=Release /p:Platform=x64 /t:AILimit
```

输出文件：`Bin\x64\Release\AILimitPlugin.dll`

第三方头文件 `nlohmann/json.hpp`（MIT）已随附于 `nlohmann/` 目录。

## 版本

- **V1.2.3** - 发布清理版（去调试日志、Go 端点留空、设置框静态布局）
- 下载：[Release AILimit_V1.2.3](https://github.com/Mariomoprc/TrafficMonitorPlugins/releases/tag/AILimit_V1.2.3)

## 许可证

MIT License
