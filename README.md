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

The existing Lite configuration excludes the temperature-monitoring subsystem. The Lite target now also uses a dedicated `CTrafficMonitorController`, so the monitor/taskbar path no longer depends on the floating-window implementation.

## Taskbar-only Lite

The Lite build is compiled with `WITHOUT_TEMPERATURE` and `TASKBAR_ONLY`.

- The floating window is not part of the Lite executable path; Lite uses a dedicated non-floating controller.
- The taskbar monitor is created immediately and is the only visible monitoring UI.
- Floating-window skin loading, positioning, background-image loading, layout, and floating-window tooltips are excluded from the Lite target.
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

The Lite target is now source-separated from the floating-window UI. `TrafficMonitorController.cpp` contains the monitor/taskbar controller used by Lite, while the floating-window controller and skin sources are explicitly excluded from all Lite configurations. Floating-window resource blocks are also omitted from the Lite resource build.

This is a build-level reduction: the excluded floating-window `.cpp` files are not compiled into the Lite executable. Exact binary-size and memory changes should be measured by the Windows CI artifacts on each release.

Implemented:

- No floating-window source/object files in Lite.
- No GDI+ startup for the floating-window skin pipeline in Lite.
- No floating-window settings page in Lite.
- Lower-frequency controller timer in Lite.
- Event-driven monitor worker instead of 10 ms idle polling.
- Existing temperature-free Lite configuration retained.

Further reductions can come from optional plugin loading, optional tray/notification support, and opt-in update checking. These should be benchmarked on real Windows builds before changing defaults.

## Releases

Release CI builds x86, x64, and ARM64EC Lite packages and publishes a GitHub Release from `version.info`.

A release is triggered by a push to `master` whose commit message contains `[release]`:

```powershell
# 1. Update the version in version.info
# 2. Commit with the release flag
git add version.info
git commit -m "Release 2 [release]"
git push origin master
```

The workflow validates the version, builds the x86, x64, and ARM64EC Lite packages, creates an **annotated `v<version>` Git tag**, and publishes the GitHub Release automatically. Release notes are generated from `version.info` release highlights and GitHub's automatic release-notes generator, and all three Lite ZIP packages are attached.

No manual tag creation or manual GitHub Release creation is required. The release contains both the complete ZIP packages and the built Lite `.exe` files as direct downloadable assets. Normal pushes to `master` do not publish a release. Pull requests run the Windows Lite build as CI validation. The workflow can also be started manually from **Actions → Release CI → Run workflow** on `master`.

Do not reuse an existing release version; the workflow stops when the `v<version>` release already exists.

## Build

The project uses Visual Studio and MFC. Existing solutions provide standard and Lite configurations for x86, x64, and ARM64EC. The Lite configurations in this fork additionally define `TASKBAR_ONLY`.

## Links

- Upstream: https://github.com/zhongyang219/TrafficMonitor
- This fork: https://github.com/j1qwerty/TrafficMonitor
- Releases: https://github.com/j1qwerty/TrafficMonitor/releases

TrafficMonitor requires the Microsoft Visual C++ runtime.

## License

See [LICENSE](./LICENSE).
