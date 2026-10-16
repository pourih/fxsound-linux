# DESKTOP — Plasma 6 / Wayland integration (Phase 9)

JUCE runs under XWayland — verify on box: `echo $XDG_SESSION_TYPE` (expect `wayland`).
Untested here; all [USER] checks below.

## 1. Autostart
- `linux/autostart_posix.{h,cpp}` writes/removes
  `~/.config/autostart/fxsound.desktop` (`Exec=fxsound --run_minimized`).
- Replaces `HKCU .../Run/FxSound` (`FxController.cpp:2789-2818`).
- [USER] Done when: login brings app up minimized with audio routed to
  "FxSound Audio Enhancer" default.

## 2. Hotkeys
- v1: `canSupportHotkeys()` returns false (see APP_LAYER.md §7).
  Document KDE custom shortcuts calling CLI through Phase 8 single-instance forward:
  - `fxsound --power=toggle` (add `--toggle` alias if wanted; `--power=on|off` exists)
  - `fxsound --preset="Name"`, `fxsound --view=1|2`, raise window (no args)
- v1.1 [USER spike]: run `tools/spike/portal_shortcut_spike.sh`
  (`org.freedesktop.portal.GlobalShortcuts`, IDs `cmd_on_off`, `cmd_open_close`,
  `cmd_next_preset`, `cmd_previous_preset`, `cmd_change_output`).
  Report whether KDE shows/binds shortcuts. Skip `XGrabKey` unless X11 targeted.

## 3. Tray
- Do NOT use JUCE `SystemTrayIconComponent` (SNI vs XEmbed differs by JUCE version;
  check `third_party/JUCE/modules/juce_gui_extra/misc/juce_SystemTrayIconComponent.cpp`
  on chosen version before trusting it).
- v1.1 target (`linux/tray_sni.{h,cpp}` stub returns false = ship v1 with no tray):
  StatusNotifierItem over sd-bus, register `org.kde.StatusNotifierWatcher`,
  expose `IconPixmap`, `Activate`, `Menu`; implement `com.canonical.dbusmenu`
  (`GetLayout`, `GetGroupProperties`, `Event`, `AboutToShow`, `LayoutUpdated`)
  so Plasma draws the menu. Never position a JUCE popup at passed coordinates.
- [USER] Done when (if built): icon visible, Plasma-rendered menu, all actions work.

## 4. Notifications
- Keep `FxNotification` + `hide_notifications` semantics; `linux/notify_posix.*`
  sends `org.freedesktop.Notifications` (gdbus fallback). If custom popup looks
  wrong under XWayland, switch to portal notifications fully.

## Results (fill on hardware)
- `XDG_SESSION_TYPE`:
- Autostart minimized + routed: YES/NO (+ journal line):
- KDE shortcut commands work: YES/NO (exact shortcut → command mapping):
- Portal spike: shows/binds/activated per ID:
- Tray icon + menu: BUILT/SKIPPED; works YES/NO:
