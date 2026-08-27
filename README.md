# TrafficMonitor-rev

[![Release CI](https://img.shields.io/github/actions/workflow/status/Andrew-Hello/TrafficMonitor-rev/main.yml?branch=master&label=Release%20CI&logo=github&style=flat-square)](https://github.com/Andrew-Hello/TrafficMonitor-rev/actions/workflows/main.yml)
[![GitHub release](https://img.shields.io/github/v/release/Andrew-Hello/TrafficMonitor-rev?style=flat-square)](https://github.com/Andrew-Hello/TrafficMonitor-rev/releases/latest)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg?style=flat-square)](./LICENSE)

TrafficMonitor-rev 是基于 [zhongyang219/TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 的增强分支，保留原版轻量、直观的任务栏资源监控体验，并针对日常 Windows 桌面使用增加经过实际测试的交互增强。

## 下载

**[前往 Releases 下载最新版本](https://github.com/Andrew-Hello/TrafficMonitor-rev/releases/latest)**

当前 Release 提供 Lite 构建：

- x64：适用于绝大多数 64 位 Windows PC
- x86：适用于 32 位环境或兼容需求
- ARM64EC：适用于 Windows on ARM

Lite 版无需管理员权限即可正常运行，推荐日常使用时保持普通用户权限运行。

## 本分支增强功能

### 任务栏鼠标滚轮调节系统音量

当鼠标指针位于 **TrafficMonitor 自己的任务栏显示窗口** 上时，可使用鼠标滚轮直接调节 Windows 系统音量。

特点：

- 功能默认关闭，可在任务栏相关设置中启用“鼠标滚轮调节系统音量”。
- 仅 TrafficMonitor 自己的任务栏窗口响应，不监听任务栏其它区域。
- 不使用全局鼠标 Hook，不向 Explorer 注入代码，不修改系统进程。
- 保留插件优先级：插件项目如果主动消费滚轮事件，则不会同时改变系统音量。
- 支持高精度滚轮/触控板滚轮增量累计，避免小增量输入丢失。
- 使用 Windows 原生 `WM_APPCOMMAND` / `APPCOMMAND_VOLUME_UP` / `APPCOMMAND_VOLUME_DOWN` 路径，不依赖 `SendInput` 模拟键盘输入。
- 因此 TrafficMonitor 以普通权限运行时，即使当前前台窗口是管理员权限程序（例如提升权限的任务管理器），仍可正常调节系统音量。
- 保留 Windows 原生系统音量处理体验。

> 推荐让 TrafficMonitor 以普通用户权限运行。任务栏音量控制不需要为了兼容管理员前台窗口而提升整个 TrafficMonitor 进程的权限。

## TrafficMonitor 原有功能

- 显示当前网络上传、下载速度
- CPU、内存、CPU 频率、显卡及硬盘利用率监控
- 支持多网卡自动或手动选择
- 支持嵌入 Windows 任务栏显示
- 支持主窗口皮肤和自定义皮肤
- 历史流量统计
- 网络详细信息
- 插件系统
- 支持多显示器及任务栏显示

## 基本使用

程序启动后会显示网速监控悬浮窗。在悬浮窗或通知区图标的右键菜单中可以进入相关设置。

若需要将信息显示到任务栏，请启用“显示任务栏窗口”。任务栏窗口支持自定义显示项目、字体、颜色、宽度等属性。

任务栏窗口：

![](./Screenshots/taskbar.PNG)

主悬浮窗：

![](./Screenshots/main1.png)

皮肤：

<img src="./Screenshots/skins.PNG" style="zoom:80%;" />

## 任务栏音量控制的设计原则

这一功能刻意保持低侵入：滚轮消息只由 TrafficMonitor 自己已经存在的任务栏窗口处理，不扩展到 Windows 系统任务栏区域，也不为了实现类似第三方任务栏增强工具的全局行为而注入 Explorer。

音量调整通过 Windows Shell 的应用命令机制完成。相比通过 `SendInput` 模拟 `VK_VOLUME_UP` / `VK_VOLUME_DOWN`，该实现不会因为普通权限 TrafficMonitor 与高完整性级别前台窗口之间的 UIPI 限制而失效。

## 版本说明

本仓库目前以 **Lite** 方案作为主要构建和发布目标。Lite 版不依赖 TrafficMonitor 旧版内置的温度监控组件，因此无需管理员权限，更适合作为长期常驻任务栏程序使用。

如果需要温度监控，可参考上游项目的硬件监控插件方案：
[TrafficMonitorPlugins](https://github.com/zhongyang219/TrafficMonitorPlugins)

## 编译

项目使用 Visual Studio / MSBuild 构建。GitHub Actions 正式构建环境固定为 **Windows Server 2022 + Visual Studio 2022 / v143 + MFC**，并编译：

- `Release (lite) | x64`
- `Release (lite) | x86`
- `Release (lite) | ARM64EC`

解决方案：`TrafficMonitor_Lite.sln`

## 上游项目与文档

本项目基于原版 TrafficMonitor 开发，原项目及完整 Wiki 文档：

- [zhongyang219/TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor)
- [TrafficMonitor Wiki](https://github.com/zhongyang219/TrafficMonitor/wiki)
- [插件开发指南](https://github.com/zhongyang219/TrafficMonitor/wiki/插件开发指南)
- [本仓库 Help.md](./Help.md)
- [原版更新日志](./UpdateLog/update_log.md)

## License

本 fork 延续上游项目的许可证要求，详见 [LICENSE](./LICENSE) 与 [LICENSE_CN](./LICENSE_CN)。
