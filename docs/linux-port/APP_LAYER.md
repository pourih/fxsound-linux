# APP_LAYER — minimal Windows→Linux edits (Phase 8 guide)

Keep Windows building. New code lives in `linux/`; edits below are small
`#if JUCE_WINDOWS` guards or thin calls. Never edit `.jucer/.vcxproj/.sln`.
Untested on Linux — Linux agent verifies each item, checks `no windows.h from
FxSound Linux build`, `fxsound --status` sane, second launch forwards + exits,
presets save/reload from `~/.local/share/fxsound/presets/`.

1. **Main.cpp**
   - `:32-34` `__declspec NvOptimus/AmdPowerXpress` → `#if JUCE_WINDOWS` only.
   - `:54-60` `SetUnhandledExceptionFilter/CoInitializeEx/CoInitializeSecurity`
     → Windows only; Linux calls `fxsound_linux::installCrashHandler()`
     (`linux/crash_posix.h`).
   - `:164-272` `MiniDumpWriteDump/StackWalk64/Sym*` → Windows only.
   - `:308-321` `GetModuleFileName/SetCurrentDirectory` → delete; use
     `File::getChildFile` + `linux/paths_posix.h::factoryPresetDir()`.
   - `:684` `AttachConsole` → Windows only; Linux `--status` uses
     `writeStatusStdout()` (stdout) plus file at `statusJsonPath()`.

2. **Single instance / CLI**
   - `Main.cpp:46/136-139` already `moreThanOneInstanceAllowed()=false` +
     `anotherInstanceStarted(applyConfig)` — keep as-is on Linux (JUCE handles
     lock). Verify `--power=on|off|toggle`, preset change, raise window forward
     and second process exits without creating a sink.

3. **Settings.cpp (wincrypt)**
   - `:23-24` `<Windows.h>/<wincrypt.h>` → `#if JUCE_WINDOWS` only.
   - `.secure` (`Settings.h:32,55`, `Settings.cpp:44-46`) protects only
     hotkeys/preset/power defaults (not sensitive) → Linux plain file `0600`
     under `~/.config/FxSound/`. Never `/var/lib`.

4. **Paths**
   - `FxController.cpp:841-866,803-835,1067-1449` factory `%CWD%/Factsoft` +
     `%APPDATA%/FxSound/Presets|AutoSave` + `%DOCS%/FxSound/Presets/Export/`
     → `factoryPresetDir()/userPresetDir()/autoSaveDir()`; merged UI list,
     factory read-only, writes to user dir.

5. **FxController message window** (`FxController.h:180-217`, `.cpp:1916-2049`,
   `WM_HOTKEY/PBT_APM*/WTS*`, `RegisterClassExW/CreateWindowW`,
   `WTSRegisterSessionNotification`, `RegisterSuspendResumeNotification`)
   → hoist case bodies to methods; Linux bodies in
   `linux/message_window_posix.h` (no-ops). No logind watcher unless Phase 11
   shows no auto-recovery.

6. **FxMainWindow** (`:257,262,370,439,583` HWND/ShowWindow/SetWindowPos/
   SendMessage WM_SETICON/LoadIcon) → JUCE `setMinimised/toFront(true)/setIcon`;
   icons from BinaryData PNGs, not `resources.rc`.

7. **SysInfo** (`SysInfo.cpp:21-96` MMDeviceEnumerator, `:98-131` SCM,
   `:138-141` `GetSystemMetrics(SM_REMOTESESSION)`)
   → `pipewireConnected()`, `false`, `SystemStats`/cpuArch, `pwListSinks()`
   (`linux/sysinfo_posix.h`, `linux/os_posix.h`). `canSupportHotkeys()=false`.
