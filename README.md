# TrafficMonitor-rev

<div align="center">

**English | [简体中文](./README_CN.md)**

</div>

[![Release CI](https://img.shields.io/github/actions/workflow/status/Andrew-Hello/TrafficMonitor-rev/main.yml?branch=master&label=Release%20CI&logo=github&style=flat-square)](https://github.com/Andrew-Hello/TrafficMonitor-rev/actions/workflows/main.yml)
[![GitHub release](https://img.shields.io/github/v/release/Andrew-Hello/TrafficMonitor-rev?style=flat-square)](https://github.com/Andrew-Hello/TrafficMonitor-rev/releases/latest)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg?style=flat-square)](./LICENSE)

TrafficMonitor-rev is an enhanced fork of [zhongyang219/TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor). It keeps the lightweight and unobtrusive taskbar-monitoring experience of the original project while adding practical interaction improvements that have been tested on real Windows desktops.

## Download

**[Download the latest release](https://github.com/Andrew-Hello/TrafficMonitor-rev/releases/latest)**

Current releases provide Lite builds for:

- **x64** — recommended for most 64-bit Windows PCs
- **x86** — for 32-bit Windows or compatibility needs
- **ARM64EC** — for Windows on ARM

The Lite build works without administrator privileges. Running TrafficMonitor as a standard user is recommended for everyday use.

## Enhancements in this fork

### Control system volume with the taskbar mouse wheel

When the pointer is inside the **TrafficMonitor taskbar interaction area**, the mouse wheel can directly adjust the Windows system volume.

The interaction area is not limited to the visible TrafficMonitor window itself. This fork also covers the otherwise-unused taskbar margin directly above and below TrafficMonitor while keeping exactly the same horizontal span. As a result, users can naturally throw the pointer toward the bottom edge of the screen and still use the wheel or the configured taskbar double-click action, as long as the pointer remains horizontally aligned with TrafficMonitor.

Key details:

- The feature is **disabled by default** and can be enabled from the taskbar settings with **Mouse wheel controls system volume**.
- The extended interaction area is limited to TrafficMonitor's current real width and never expands into neighboring taskbar icons or the notification area.
- The visible TrafficMonitor window keeps its original size, layout, and rendering. The window is not stretched merely to enlarge the hit area.
- Double-clicks in the surrounding taskbar margin reuse TrafficMonitor's existing taskbar double-click action.
- No global mouse hook is installed, no code is injected into Explorer, and no system process is modified.
- Windows Raw Input is used only as an observation source for the small taskbar gutter area.
- For each relevant mouse event, the current screen rectangles of the TrafficMonitor HWND and taskbar HWND are read again. No cached position is trusted, allowing the interaction area to follow icon-count changes, TrafficMonitor width changes, DPI changes, taskbar resizing, and display changes.
- `WindowFromPoint` is used to verify that the taskbar is actually underneath the pointer, reducing accidental activation when another window covers the taskbar.
- Plugin priority is preserved inside the visible TrafficMonitor window: if a plugin item consumes the wheel event, system volume is not changed at the same time.
- High-resolution wheel and touchpad wheel deltas are accumulated so that small input increments are not lost.
- Volume changes use the native Windows `WM_APPCOMMAND` / `APPCOMMAND_VOLUME_UP` / `APPCOMMAND_VOLUME_DOWN` path instead of simulated `SendInput` media keys.
- Because the implementation does not rely on `SendInput`, a standard-user TrafficMonitor can still change system volume even when the foreground application is elevated, such as Task Manager running as administrator.
- Native Windows volume handling and feedback are preserved.

> **Recommended:** keep TrafficMonitor running as a standard user. The taskbar volume-control feature does not require administrator privileges, even when the foreground application is elevated.

## Original TrafficMonitor features

- Current upload and download speed
- CPU and memory usage
- CPU frequency, GPU usage, and disk utilization
- Automatic or manual network-adapter selection
- Embedded Windows taskbar display
- Main-window skins and custom skins
- Historical traffic statistics
- Detailed network information
- Plugin system
- Multi-monitor and multi-taskbar support

## Basic usage

After launch, TrafficMonitor displays the floating network-speed monitor. Settings are available from the context menu of the floating window or notification-area icon.

To display information in the Windows taskbar, enable **Show taskbar window**. The taskbar window supports configurable items, fonts, colors, widths, and other appearance options.

Taskbar window:

![](./Screenshots/taskbar.PNG)

Main floating window:

![](./Screenshots/main1.png)

Skins:

<img src="./Screenshots/skins.PNG" style="zoom:80%;" />

## Design principles of taskbar volume control

This feature is intentionally designed to remain low-impact. The visible TrafficMonitor taskbar window continues to be embedded using its original dimensions and rendering behavior. For the small taskbar margin that the visible window does not cover, Raw Input is used only to observe mouse activity, followed by strict geometric checks against the current real HWND coordinates.

No transparent overlay is placed over Explorer. No global mouse hook or code injection is used to alter taskbar behavior.

Raw Input is received by a message-only window and does not suppress or intercept normal Windows mouse input. Additional interaction is triggered only when all of the following are true:

- the pointer is currently inside the taskbar;
- its horizontal position is strictly within TrafficMonitor's current real width;
- it is outside the visible TrafficMonitor HWND but inside the corresponding taskbar margin; and
- the taskbar hierarchy is actually underneath the pointer.

The final volume adjustment is still performed through the Windows Shell application-command mechanism. Compared with simulating `VK_VOLUME_UP` / `VK_VOLUME_DOWN` through `SendInput`, this avoids failures caused by UIPI when a standard-integrity TrafficMonitor is used while a high-integrity application is in the foreground.

## Build and release policy

This repository primarily publishes the **Lite** build. The Lite version does not depend on TrafficMonitor's legacy built-in temperature-monitoring component, so it does not require administrator privileges and is better suited to running continuously in the taskbar.

If you need hardware temperature monitoring, consider the upstream hardware-monitoring plugin ecosystem:
[TrafficMonitorPlugins](https://github.com/zhongyang219/TrafficMonitorPlugins)

The project is built with Visual Studio / MSBuild. Official GitHub Actions releases use **Windows Server 2022 + Visual Studio 2022 / v143 + MFC** and build:

- `Release (lite) | x64`
- `Release (lite) | x86`
- `Release (lite) | ARM64EC`

Solution: `TrafficMonitor_Lite.sln`

## Upstream project and documentation

TrafficMonitor-rev is based on the original TrafficMonitor project. For the upstream project and its full documentation, see:

- [zhongyang219/TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor)
- [TrafficMonitor Wiki](https://github.com/zhongyang219/TrafficMonitor/wiki)
- [Plugin development guide](https://github.com/zhongyang219/TrafficMonitor/wiki/插件开发指南)
- [Help.md in this repository](./Help.md)
- [Original update log](./UpdateLog/update_log.md)

## License

This fork follows the licensing requirements of the upstream project. See [LICENSE](./LICENSE) and [LICENSE_CN](./LICENSE_CN) for details.
