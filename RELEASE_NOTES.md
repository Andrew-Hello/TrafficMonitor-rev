# TrafficMonitor Rev v1.86-rev.1

本版本在 TrafficMonitor Lite 基础上增加了任务栏鼠标滚轮系统音量控制，并针对普通权限运行场景进行了实际 Windows 桌面测试。

## 新增

- 鼠标位于 TrafficMonitor 自己的任务栏窗口上时，可通过滚轮调节 Windows 系统音量。
- 在任务栏设置中新增“鼠标滚轮调节系统音量”开关，默认关闭。
- 支持高精度滚轮增量累计，改善高分辨率滚轮和触控板输入体验。
- 保留插件优先级：插件项目主动处理滚轮事件时，不会同时改变系统音量。

## 实现与稳定性

- 使用 Windows 原生 `WM_APPCOMMAND` / `APPCOMMAND_VOLUME_UP` / `APPCOMMAND_VOLUME_DOWN` 调节音量。
- 不再依赖 `SendInput` 模拟媒体键，因此不会因为普通权限 TrafficMonitor 与管理员权限前台窗口之间的 UIPI 隔离而失效。
- 已验证：TrafficMonitor 以普通用户权限运行、管理员权限任务管理器处于前台时，任务栏滚轮音量控制仍可正常工作。
- 不使用全局鼠标 Hook。
- 不向 Explorer 注入代码。
- 不修改系统进程。

## 运行建议

推荐使用 Lite 版并保持普通用户权限运行。该音量控制功能不需要管理员权限。

## 构建

本 Release 提供：

- x64 Lite
- x86 Lite
- ARM64EC Lite

正式构建环境为 Windows Server 2022 + Visual Studio 2022 / v143 + MFC。
