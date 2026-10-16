# RECON — FxSound Linux port (Fedora 44 KDE / PipeWire)

Branch: `linux` @ `d8e7a23`. Submodule `Resources/` initialized (`8f0385f`).
No code changed for this doc except include-case fixes in separate commit.
Untested on Linux — do not claim tested.

## 1. License

- App `LICENSE:1-2` = **GNU AFFERO GENERAL PUBLIC LICENSE Version 3** → AGPL-3.0-or-later (headers say “version 3 … or any later version”). NOT GPLv3. Earlier plans saying AGPL-3.0 were correct; plan note “may be GPLv3” is wrong.
- Headers sampled: `dsp/DfxDsp.cpp:1-20`, `dsp/include/DfxDsp.h:1-19`, `dsp/ptutil/include/dfxSharedUtil.h:1-20`, `pt_defs.h:1-19`, `platform.h:1-17`, `audiopassthru/include/AudioPassthru.h:1-17`, `u_AudioPassthru.h:1-17`, `fxsound/Source/Main.cpp:1-17` — all AGPL-3.0-or-later `FxSound LLC 2025`. Theremino contributor line in `dsp/DfxDsp.cpp:5-6`, `dsp/include/DfxDsp.h:4-5`, many `ptutil/include/*.h`, `FxController.cpp:6`, `Settings.cpp:6`, `README.md:52`.
- Legacy: `dsp/ptutil/Qnt/u_qnt.h:1-5` = `(C) 1994-1997 Power Technology. All Rights Reserved … without express written consent` — only pre-FxSound header found; full-tree legacy audit UNSURE.
- JUCE: `README.md:22-23` = `JUCE 6.1.6` for x64/x86 + `Latest JUCE` for ARM64. `fxsound/FxSound.jucer:3` + `FxSoundARM.jucer:3` `version="1.2.15.0"` is app version, not JUCE. No JUCE pin in `.jucer`; `MODULEPATHS C:\JUCE\modules` absolute host path. No JUCE submodule (`.gitmodules` only `Resources`). `README.md:50` claims JUCE under AGPL v3.0. Implication: distributed binary = AGPL-3.0-or-later + source offer. Whether upstream 6.1.6 is GPLv3 vs AGPLv3 vs commercial is UNSURE from repo alone. `fxsound/JuceLibraryCode/` exists but JUCE sources not vendored — exact checkout UNSURE.

## 2. Case and paths

Disk has capitals that break on case-sensitive FS:
- `dsp/ptutil/include/File.h`, `Prelst.h`, `Pwav.h`, `GraphicEq.h`, `SurroundSyn.h`, `BinauralSyn.h`, `DfxSdk.h`, `U_vals.h` vs `u_qnt.h`/`u_sos.h` etc.
- `audiopassthru/include/File.h`, `src/FILE/U_FILE.H`, `src/MRY/U_MRY.H`, `src/reg/U_reg.h`, `dsp/ptutil/PWAV/U_pwav.h`, `PRELST/U_prelst.h`, `dsp/u_DfxDsp.h`
- `dfxpdefs.h` (audiopassthru, lower) vs `dfxpDefs.h` (dsp camel) — different files, collide case-insensitively — UNSURE include-path winner.

Wrong-case `#include` (fails on Linux, fixed in housekeeping commit):
- `dsp/DfxDspPrivate.cpp:25-26` `"prelst.h"`→`Prelst.h`, `"file.h"`→`File.h`; `:31` `"ptutil\dfxp\u_dfxp.h"` backslash → forward slash
- `dsp/ptutil/dfxp/dfxpProcessClear.cpp:36`, `dfxpRegistryStandard.cpp:37`, `dfxpSpectrum.cpp:33`, `dfxSharedUtil.cpp:27`, `PRELST/Prelst.cpp:32`, `VALS/Valscfg.cpp:30`, `Valsfile.cpp:30`: `"file.h"`→`File.h`
- `PRELST/Prelst.cpp:33` `"prelst.h"`→`Prelst.h`; `:36` `"u_prelst.h"`→`U_prelst.h`
- `PWAV/Pwav.cpp:31`, `PwavConvert.cpp:32`, `PwavIO.cpp:31`, `PwavUtil.cpp:27`, `U_pwav.h:26`, `COM/Comwave.cpp:50`: `"pwav.h"`→`Pwav.h`; plus `"u_pwav.h"`→`U_pwav.h` in same files
- `VALS/Vals*.cpp`: `"u_vals.h"`→`U_vals.h`
- `audiopassthru/src/FILE/FileDate.cpp:22-23`, `FileGeneral.cpp:32,40`, `fileWin32Handle.cpp:22-23`: `"file.h"`→`File.h`, `"u_file.h"`→`U_FILE.H`
- `MRY/Mry.cpp:34` `"u_mry.h"`→`U_MRY.H`; `reg/*.cpp` `"u_reg.h"`→`U_reg.h`; `ptime/Ptime.cpp:32` `"Ptime.h"`→`ptime.h`
- `dsp/Win32Main/Win32Main.rc:3` `"resource.h"`→`Resource.h`

Backslash path strings (runtime, NOT fixed here — need POSIX siblings in Phase 5/8):
- `fxsound/Source/GUI/FxController.cpp:717` `L"Software\\DFX"`, `:740,806,811,857,1215,1225,1256,1386,1400,1435` `L"FxSound\\Presets…"`, `L"\\"`, `:2792,2806` Run key; `FxPresetNameEditor.cpp:14`, `FxPresetExportDialog.cpp:196`, `Main.cpp:222,291` `strrchr(...,'\\')` (`Main.cpp:315` `find_last_of(L"\\/")` is portable)
- `dsp/DfxDspPreset.cpp:106,127` `.append(L".fac")` portable; `Win32Main.cpp:335-348` hardcoded `C:\Program Files (x86)\DFX\…\1.fac` test-harness only
- `audiopassthru/src/FILE/FileGeneral.cpp:703,716,725,841`, `sndDevicesSet.cpp:587-588,629-630` globs, `sndDevicesReg.cpp:56,62,98,104` registry `\\`, `operatingSystem.cpp:89,102`, `pstr*.cpp` `L'\\'` handling — file/registry separators, replaced by POSIX layer later.

## 3. Non-portable constructs

- `__asm`: 0 hits in `dsp/,audiopassthru/,fxsound/Source/,fxdiag/`. `__forceinline`: 0 hits.
- `__declspec`: `dsp/ptutil/include/c_dsps.h:28,30` (`DSP_FUNC_DEF`), `comSftwrCPP.h:19,25`, `comSftwr.h:46,52`, `hrdwr.h:17,23`, `codedefs.h:60,65` (+ audiopassthru twin), `audiopassthru/include/BladeMP3EncDLL.h:30-31,258-268,277`, `fxsound/Source/Main.cpp:32,34` (`NvOptimusEnablement`, `AmdPowerXpressRequestHighPerformance`).
- `#pragma pack`: only `BladeMP3EncDLL.h:258-268` (`BE_CONFIG/BE_VERSION`); `#pragma comment`: `Main.cpp:28` (`dbghelp.lib`).
- `windows.h` includes: ~60 files. Lower `windows.h`: `dsp/ptutil/dfxp/dfxp*.cpp:23`, `PWAV/Pwav*.cpp`, `audiopassthru/src/sndDevices/*.cpp:23`, `pstr/*.cpp:21`, `FILE/*.cpp`, `operatingSystem/*.cpp`, `MTH/MthRand.cpp:21`. Capital `<Windows.h>`: `audiopassthru/include/pstr.h:41`, `AudioPassthru.h:26`, `BladeMP3EncDLL.h:76`, `fxdiag.h:1`, `U_vals.h:21`, `pstr.h:38`, `Settings.cpp:23`, `DfxInstall.h:21`, `dfxpUniversal.cpp:26`.
- `GetModuleFileName/SetCurrentDirectory`: `Main.cpp:311,318` (exe-dir CWD hack for `Factsoft/`), `FxController.cpp:2804` (autostart Run value).
- `Sleep(`: `COM/Com.cpp:279`, `sndDevicesReInit.cpp:102`, `sndDevicesDoCapture.cpp:99`, `FileGeneral.cpp:139`, `AudioPassthruPrivate.cpp:313`.
- `WIN32/_WIN32_WINNT/WIN32_LEAN_AND_MEAN`: `DfxDsp.vcxproj:118,165`, `Win32Main.vcxproj/stdafx.h/targetver.h`, `ComsftwrCPP.cpp/c:32+`, `COM/Com.cpp:20`, `codedefs.h:32,37-38,121,138,141,156`, `MthUtil.cpp:35,427,627`, `Slout.cpp:125,188`, `MthRand.cpp:123,140`.
- `_stricmp/_wcsicmp/wcscpy_s/_wfopen_s/swprintf_s`: `Pwav.cpp:871`, `pstrArray.cpp:197,226,287,393`, `pstrArraySort.cpp:86`, `sndDevices_GetAll.cpp:155`, `FxController.cpp:1260,1291,1357,2911`, `FxSystemTrayView.cpp:79`, `FileGeneral.cpp:86,140,720`, `Valscfg.cpp:206`, `fxdiag.cpp:37`, `fxdevcon.cpp:35,55`. `_strnicmp`: 0 hits.
- `long` in serialized structs: `U_vals.h:40` (`sum_scale_val:long`), `valsCfgHdlType.total_samples/preset_save_time:long`, `U_pwav.h:58,75`, `u_comSftwr.h:44-45,54-57`, `Comsftwr.c:69,109,190,278,292`, `c_*.h` ~20 `long` flags each, `com.h` ~15 APIs, `Pwav.h:24-26,44-54`. VALS uses `%d/%g/%ld` text (not binary dump) but `%ld` truncates/extends LP64 (Windows 4B vs Linux 8B). Fix: fixed-width types in Phase 6.
- `wchar_t` in `.fac` I/O: all of `DfxDspPreset.cpp` (wchar paths, `createValsFromStateInfo(wchar_t*)`), `DfxPreset{std::wstring}`, `U_vals.h/U_pwav.h/U_prelst.h wcp_*`, `dfxpSession` `wcp_*`. `.fac` = VALS text (`Valsfile.cpp:45-187,266-507`: `swprintf L"%s\\%s"`, `fileOpen_Wide "w"/"r"`, `fwprintf L"%g/%d/%s"`, comment UTF-8 via convert). Risks: `wchar_t` 2B vs 4B, `fwprintf %s` locale-dependent, mixed UTF-8 comment + locale strings, `LINE_LENGTH 128` truncation. Fix: `char16_t`/UTF-8 helpers in Phase 6. `char16_t`: 0 hits currently.
- `PWAV` (`Pwav*.cpp`, `U_pwav.h:21-24` `#ifndef __ANDROID__ <windows.h>+<mmsystem.h>`): WinMM `mmioRead/HMMIO/WAVEFORMATEX/HWAVEIN` — needs stub/ALSA replace; UNSURE if used at runtime (looks legacy file-play test).

## 4. Registry — keys and when

Wrappers `audiopassthru/src/reg/regWithKeyname.cpp`, `regWithoutKeyname.cpp`, `regRecursiveDelete.cpp`, `U_reg.h` (`REG_*` ids) → `RegCreateKeyExW/RegOpenKeyExW/RegSetValueExW/RegQueryValueExW/RegDeleteKeyW`.

- `SOFTWARE\DFX\<ver=13>\<vendor=23>\devices\<value>` HKCU: `user_selected_playback,most_recent_playback,most_recent_default,prior_default,original_default` — read `sndDevicesSetupDevices.cpp:583-611`, `ImplementDeviceRules.cpp:111-139`, `Get.cpp:70,126`; written `ImplementDeviceRules.cpp:156,158,364,371,375` (startup + device-change = both startup and runtime).
- Same path HKCU `default_device_mode` — write `sndDevicesSet.cpp:274,279`, read `Get.cpp:652` (runtime user toggle).
- Same path HKCU `user_buffer_size` — write `Set.cpp:552`, read `ReInit.cpp:236`, `Get.cpp:682` (runtime + startup sizing).
- Same path HKLM `dfx_guid,default_buffer_size` — read `Set.cpp:313,395`, `Get.cpp:693`, `ReInit.cpp:246`; never written by app (installer/driver owned).
- `...LASTUSED_DFXP\<valFidelity…/byFidelity…/EQ/on/…>` HKCU — `dfxpSession.cpp:75,126,177,224` (`Write/ReadInteger/RealValue`), called `dfxpSet.cpp:99-433` (every slider/bypass), `DfxDspEq.cpp:63,148,161,280,315`, `dfxpProcess.cpp:205,240`, `dfxpGet.cpp:103+` (runtime).
- `...LASTUSED_DFXG\…` HKCU — `DfxDspRegistry.cpp:47` dead (caller commented `DfxDspPreset.cpp:75`).
- `SOFTWARE\DFX\…\Shared\top_shared_folder/top_folder/dfxui_path/date_installed` HKLM — read `dfxpRegistryStandard.cpp:88,229,266,301` (install-path discovery, startup).
- `HKCU\Software\DFX` whole tree — delete `FxController.cpp:717 RegDeleteTree` (factory-reset button, runtime).
- `HKCU\Software\Microsoft\Windows\CurrentVersion\Run\FxSound` — read `:2792-2796` startup, write/delete `:2806-2817` on settings toggle (both).
- `SOFTWARE\Microsoft\Active Setup\…` (`operatingSystem.cpp:89,102`) WMP check; `AppEvents\…\.Current` (`operatingSystemSounds.cpp:83`) system sounds.
- `dfxpdefs.h:40-97,218-219`, `sndDevices.h:159-170`, `reg.h:34-38`, `pt_defs.h:53-63`, `operatingSystem.h:44,46`, `DfxDspRegistry.cpp:23-25` define all names.

## 5. Networking (`JUCE_USE_CURL` decision)

- Only in-app fetch: `fxsound/Source/MainComponent.cpp:196` `WebInputStream stream(URL(CHECK_ANNOUNCEMENT_URL),false)` with `MainComponent.h:23-24` `CHECK_ANNOUNCEMENT_URL=https://www.fxsound.com/tmp/check_announcement.json` (blocking in `checkAnnouncement`). Needs `juce_core` networking + TLS. No `Http/curl/JUCE_USE_CURL/URL::` elsewhere in `fxsound/Source`.
- External-browser links only (fine via JUCE/`xdg-open`): `FxController.cpp:64,73`, `FxMainWindow.cpp:215-216,490-491,479-481,545`, `FxMessage.cpp:67`, `FxNotification.cpp:78,185`, `FxSettingsDialog.cpp:481,485,487`, `FxSystemTrayView.cpp:281`, `FxView.cpp:63`.
- Updater is Windows-only stub: `FxController.cpp:2624` + `FxMainWindow.cpp:486` (`updater.exe /silent|/checknow`), `Installer/fxsound.aip:359-369` (`updater.ini URL=https://github.com/…/release/updates.txt`), `fxsound_update.aip` (`URL=…/fxsound_setup.exe`), `DfxInstall.cpp:322-323,340` (`schtasks FxSound\Update`), `release/updates.txt:6-7,13`. Drop/replace on Linux. `fxmcp/` has zero network hits (local stdio).
- Decision: set `JUCE_USE_CURL=1` on Linux (needs `libcurl-devel`) for `WebInputStream` TLS; else JUCE falls back to raw sockets. Recorded in DECISIONS.md.

## 6. JUCE (`juce_opengl`, `OpenGLContext`)

- `fxsound/Source/` grep `OpenGLContext|juce_opengl|OpenGL`: 0 hits (only `NvOptimus` comments).
- Both `.jucer` list `juce_opengl` module (`FxSound.jucer:295-304`, `FxSoundARM.jucer:300-309`) alongside 7 others, `useGlobalPath`. Generated `ProjectARM/*.vcxproj` compiles `juce_opengl.cpp`, `OpenGLContext/FrameBuffer/…` + `juce_core/network/juce_WebInputStream.cpp` — linked but never referenced by app code.
- Verdict: `juce_opengl` declared but unused — safe to drop on Linux (avoids `libGL`). Keep `juce_core` networking.

## 7. Single-instance

- `fxsound/Source/Main.cpp:46` `moreThanOneInstanceAllowed(){return false;}` + `:136-139` `anotherInstanceStarted(commandline){FxController::getInstance().applyConfig(commandline);}` — JUCE gate + CLI forward. No `CreateMutex/FindWindow` in `fxsound/Source`. `FxController.h:180-217` `MessageWindow` (`RegisterClassExW/CreateWindowW`) is hotkey/power window, NOT single-instance. `Win32Main.cpp:106-125,141` window is test-harness only. How JUCE enforces (lockfile/socket on Linux) UNSURE from tree.

## 8. DSP interface

- Public: `dsp/include/DfxDsp.h:37-73` (29 methods; see §4 of recon task). Key: `:44` `processAudio(short*,short*,int num_sets,int check_dup)` — **no float variant** (mirrored `u_DfxDsp.h:83-84`, `u_AudioPassthru.h:32-33`, impls `DfxDsp.cpp:95-114`, `DfxDspPrivate.cpp:181-189`). Caller casts float WASAPI buffer: `AudioPassthruPrivate.cpp:551` `(short*)fp_buffer`. Internal comment `DfxDspPrivate.cpp:184` + `dfxpUniversal.cpp:63-64` says 32-bit float inside — exact conversion site UNSURE.
- Sample rates: pass-through WASAPI (`AudioPassthruPrivate.cpp:533` `setSignalFormat(bps,nch,srate,…)`); device layer forces 44.1/48k (`sndDevicesSet.cpp:490-493`, `ImplementDeviceRules.cpp:419-427,438`); DSP consts `u_dfxp.h:43,45` (`44100.0`, `48000.0 max`), `BinauralSyn.h:24,26`, `Com.cpp:402-403` (`44100/44056/48000`). 88.2/96/192k UNSURE.
- Block size: variable per-callback (`sndDevicesDoCapture.cpp:47,84,385`, `Get.cpp:577,588`); DSP chunks `>16384` (`u_dfxp.h:38,56`, `dfxpUniversal.cpp:152-163`, `dfxpProcessReal.cpp:109`, `dfxpProcessInt.cpp:82`). WASAPI sizing `sndDevices.h:194-201` (40-100ms, MIN 10/MAX 100ms) + registry override `ReInit.cpp:233-262`. Linux frames UNSURE (WASAPI-specific).
- Delay: no end-to-end spec — UNSURE. Internals: `c_max.h:47,49` (96 max, 0.75ms look-ahead), `c_play.h:133-155` (up to 36ms taps), `c_lex.h:60-63` (100ms predelay max).
- `dsp/Win32Main/`: GUI test harness, NOT CLI (`Win32Main.cpp:55-97` `wWinMain` ignores CLI, buttons Power/EQ/Preset 1/2/3 with hardcoded `C:\…\DFX\…\1.fac`). Phase 4 must build new `dsp_cli`.

## 9. Preset locations (Windows)

- Factory: `FxController.cpp:841-854` (`File::getCurrentWorkingDirectory()` + `L"Factsoft"` + `"*.fac"` → `getPresetInfo`). Depends on `Main.cpp:308-321` exe-dir `SetCurrentDirectory` + README debug dir `$(SolutionDir)..\..\bin\$(PlatformTarget)`. Linux: `/usr/share/fxsound/Factsoft` + exe-adjacent fallback (Phase 8).
- User: `FxController.cpp:856-866` (`%APPDATA% + L"FxSound\\Presets"` → `UserPreset`); writes `:1215-1226,1256-1257,1421-1435`, autosave `:803-812,822-835` (`%APPDATA%/FxSound/AutoSave`), export `:1384-1400` (`%USERDOCS%/FxSound/Presets/Export/`), import `FxPresetImportDialog.cpp:261,270`. `FxModel` in-memory only. `Settings/DeviceConfig` have no preset paths. `commonApplicationDataDirectory`: 0 hits. Linux: `$XDG_DATA_HOME/fxsound/presets/` (`~/.local/share/…`), factory read-only merged (Phase 8).
