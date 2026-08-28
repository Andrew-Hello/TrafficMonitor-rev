# TrafficMonitor Rev v1.86-rev.2

本版本进一步完善任务栏鼠标交互区域，解决 TrafficMonitor 可见任务栏窗体没有覆盖完整任务栏高度时，上下空白余量无法响应滚轮和双击的问题。

## 改进

- 当鼠标位于 TrafficMonitor 当前实际宽度对应的任务栏上下余量区域时，滚轮仍可正常调节 Windows 系统音量。
- 任务栏余量区域中的双击会复用 TrafficMonitor 原有的任务栏双击动作，例如打开任务管理器或用户配置的其它动作。
- 可见 TrafficMonitor 窗口的尺寸、文字布局和绘制方式保持不变，不通过拉伸窗体来扩大鼠标命中区域。
- 交互范围只沿任务栏垂直方向补齐，不扩展到左右相邻应用图标或通知区域。

## 实现与稳定性

- 使用 Windows Raw Input 作为鼠标事件观察来源，不使用全局鼠标 Hook，不向 Explorer 注入代码，也不修改系统进程。
- Raw Input 不拦截或屏蔽系统鼠标输入；程序只在严格命中 TrafficMonitor 对应任务栏余量区域时执行补充交互。
- 每次相关鼠标事件都重新通过 `GetWindowRect` 获取 TrafficMonitor HWND 与任务栏 HWND 的当前屏幕坐标，不依赖位置缓存，从而可以跟随图标数量、TrafficMonitor 宽度、DPI、任务栏尺寸及显示器变化导致的位置变化。
- 使用 `WindowFromPoint` 确认光标下方当前实际仍属于任务栏层级，减少其它窗口覆盖任务栏时的误触发。
- 音量调整仍沿用已经验证的 Windows 原生 `WM_APPCOMMAND` / `APPCOMMAND_VOLUME_UP` / `APPCOMMAND_VOLUME_DOWN` 路径，不依赖 `SendInput`。
- 已在 Windows 11 实机验证：此前位于可见 TrafficMonitor 窗体下方的任务栏空白区域，现在可以正常触发滚轮音量控制及双击动作。

## 运行建议

推荐继续使用 Lite 版并保持普通用户权限运行。上述任务栏交互功能不需要管理员权限。

## 构建

本 Release 提供：

- x64 Lite
- x86 Lite
- ARM64EC Lite

正式构建环境为 Windows Server 2022 + Visual Studio 2022 / v143 + MFC。
