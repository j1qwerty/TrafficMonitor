# TrafficMonitor

TrafficMonitor is a Windows system monitor for real-time network speed and system resource usage. This fork makes English the default language and adds a taskbar-only Lite build that avoids creating the floating-window UI.

## Features

- Real-time upload and download speed monitoring.
- CPU usage and memory usage monitoring with percentage, used-memory, and available-memory display modes.
- CPU frequency, GPU usage, and hard-disk usage monitoring.
- Optional CPU, GPU, hard-disk, and motherboard temperature monitoring in the non-Lite build.
- Automatic, manual, or all-interface network adapter selection.
- Hidden-interface filtering, connection refresh, and detailed network information.
- Taskbar embedding with Windows 7/8/10/11-specific handling and Wine support.
- Secondary-display taskbar placement and DPI-aware positioning.
- Custom taskbar item order, labels, fonts, colors, transparency, spacing, alignment, margins, and display units.
- Upload, download, CPU, memory, GPU, disk, CPU frequency, total speed, today's traffic, and plugin display items.
- Byte/bit units, automatic/fixed speed units, hidden units, hidden percentage signs, and compact speed formatting.
- CPU/memory status bars and network-speed graphs with bar or plot modes.
- Light/dark theme adaptation and automatic taskbar background-color handling.
- Tooltips and configurable double-click actions.
- Historical traffic statistics with day, week, month, quarter, and year views.
- Incremental current-day history saving to reduce frequent disk I/O.
- System-tray integration with theme-aware tray icons and tooltip information.
- Traffic, memory, hardware, and plugin notifications where applicable.
- Configurable floating-window skins, per-skin fonts/colors/text/layout, skin auto-adaptation, BMP/PNG backgrounds, and INI/XML skin configuration in the full build.
- Plugin DLL loading with display items, commands, menus, options, configuration, enable/disable management, notifications, tooltips, and mouse/keyboard callbacks.
- Automatic update checks with GitHub or Gitee sources.
- Windows startup support through Registry or Task Scheduler.
- Portable or AppData-based configuration.
- Configurable monitoring interval and CPU-usage acquisition method.
- Multi-language packs with English as the default language in this fork.
- Crash reporting, diagnostic logging, and rendering fallback support.
- DPI-aware UI behavior and Windows 11 taskbar placement/overlap options.

## Standard and Lite builds

| Build | Floating window | Taskbar | Temperature monitoring |
| --- | --- | --- | --- |
| Standard | Yes | Yes | Yes |
| Lite | No | Yes | No |

The existing Lite configuration already excludes the temperature-monitoring subsystem. This fork additionally defines `TASKBAR_ONLY` for Lite so the floating window is not created or initialized at runtime.

## Taskbar-only Lite

The Lite build is compiled with `WITHOUT_TEMPERATURE` and `TASKBAR_ONLY`.

- The floating window is not shown or initialized; a hidden controller remains only because the current architecture shares monitoring/controller code with the taskbar window.
- The taskbar monitor is created immediately and is the only visible monitoring UI.
- Floating-window skin loading, positioning, background-image loading, layout, and floating-window tooltips are skipped.
- The floating-window settings tab is omitted from the Lite Options dialog.
- The taskbar menu does not expose controls for showing/hiding the floating window or closing the taskbar monitor.
- The controller taskbar-management timer runs at 1 second instead of the full build's 100 ms timer.
- The monitor worker waits on an event instead of polling every 10 ms while idle.
- GDI+ initialization for the floating-window skin pipeline is skipped.
- The system tray remains available for notifications and recovery.

## Network monitoring

- Automatically select the active network adapter.
- Manually select a specific adapter.
- Aggregate all network adapters.
- Hide selected adapters from selection lists.
- Refresh the network-adapter list.
- View detailed connection information.

## Taskbar display customization

Taskbar output supports upload/download, CPU, memory, GPU usage, disk usage, CPU frequency, total speed, today's traffic, and plugin-provided display items. Users can customize ordering, labels, colors, fonts, transparency, spacing, layout, units, compact formatting, status bars, and network-speed graphs.

Windows 11-specific taskbar options include placement offsets, snapping close to taskbar icons, and avoiding overlap with right-side widgets.

## History

Traffic history is persisted to disk and can be viewed by day, week, month, quarter, or year. The current day's record is saved incrementally to reduce unnecessary I/O.

## Skins

The full floating-window build supports built-in and custom skins, per-skin fonts, text colors, display strings and layout, light/dark skin adaptation, BMP/PNG backgrounds, and INI/XML skin configuration.

The taskbar-only Lite build does not initialize the floating-window skin pipeline.

## Plugins

Plugins are DLLs loaded from the `plugins` directory. The plugin system supports additional display items, commands and menus, plugin-specific options/configuration, enable/disable management, tooltips, mouse/keyboard events, notifications, and monitor-data callbacks.

## Notifications and system tray

TrafficMonitor supports system-tray icons, light/dark-aware icon selection, traffic and memory threshold alerts, hardware alerts where hardware monitoring is available, plugin notifications, and tooltip information.

## Configuration

- English is the default language for a fresh configuration; explicit existing language selections remain supported.
- Portable mode or AppData configuration.
- Configurable monitoring interval.
- CPU usage acquisition by CPU-time or PDH.
- Automatic update checking with GitHub/Gitee source selection.
- Registry or Task Scheduler startup.
- Configurable notification intervals and thresholds.

## Performance and size

This fork targets runtime CPU/memory overhead in the Lite build. The current shared-controller architecture means the floating-window source code and resources are still part of the project, so this PR does not claim a measured executable-size reduction.

Implemented:

- No floating-window creation or runtime initialization in Lite.
- No GDI+ startup in Lite.
- No floating-window settings page in Lite.
- Lower-frequency controller timer in Lite.
- Event-driven monitor worker instead of 10 ms idle polling.
- Existing temperature-free Lite configuration retained.

For a second-stage size reduction, the shared controller should be split into a monitor service and taskbar UI so floating-window source files and resources can be excluded from the Lite target entirely. Other possible opt-in reductions are lazy plugin initialization, optional tray/notification support, and opt-in update checking for an ultra-minimal profile. These should be benchmarked on real Windows builds before changing defaults.

## Build

The project uses Visual Studio and MFC. Existing solutions provide standard and Lite configurations for x86, x64, and ARM64EC. The Lite configurations in this fork additionally define `TASKBAR_ONLY`.

## Links

- Upstream: https://github.com/zhongyang219/TrafficMonitor
- This fork: https://github.com/j1qwerty/TrafficMonitor
- Releases: https://github.com/j1qwerty/TrafficMonitor/releases

TrafficMonitor requires the Microsoft Visual C++ runtime.

## License

See [LICENSE](./LICENSE).
