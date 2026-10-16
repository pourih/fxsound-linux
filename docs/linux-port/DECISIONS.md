# DECISIONS — FxSound Linux port

Log every decision with one-line reason. Untested on Linux.

- 2026-10-16: Branch `linux` from `main@d8e7a23`, init `Resources@8f0385f` — need BinaryData strings.
- 2026-10-16: License = AGPL-3.0-or-later (not GPLv3) — `LICENSE:1-2` + headers say AGPL.
- 2026-10-16: JUCE rule → try 6.1.6 first (`.jucer` pins it, no shared drift) — fallback 7.0.12 if >~10 GCC16 patches, 8.x last resort; submodule exact commit, no FetchContent.
- 2026-10-16: Drop `juce_opengl` on Linux — 0 uses in `fxsound/Source/`, only linked.
- 2026-10-16: `JUCE_USE_CURL=1` on Linux — only fetch is `MainComponent.cpp:196 WebInputStream` announcement JSON, needs TLS.
- 2026-10-16: Streams F32 interleaved 48k/2ch, DSP inline (not worker) by default — per final plan review; `RT_AUDIT.md` verdict can switch to worker.
- 2026-10-16: No `win_compat.h` force-include — `compat/` only for `DfxDsp/ptutil/AudioPassthruCore`, never PipeWire code; split `AudioPassthruCore` vs `AudioPassthruPW`.
- 2026-10-16: Registry → `~/.config/FxSound/` settings file; `dfx_guid` = sink `node.name`; startup-only keys = logged compile defaults.
- 2026-10-16: Factory `/usr/share/fxsound/Factsoft` + exe-adjacent fallback; user `$XDG_DATA_HOME/fxsound/presets/` merged read-only factory — XDG + read-only system dir.
- 2026-10-16: Previous default in `~/.local/state/fxsound/previous_sink`, restore only if still fxsound — never overwrite user choice.
- 2026-10-16: `canSupportHotkeys()=false` v1, KDE shortcuts call `fxsound --power=toggle` via single-instance forward — portal spike later.
- 2026-10-16: Tray v1 may ship without tray; v1.1 = SNI + dbusmenu, no JUCE popup coordinates (Wayland forbids) — verify SNI vs XEmbed on chosen JUCE.
- 2026-10-16: Out of scope v1: `Installer/`, `fxmcp`, `fxdiag` (use `pw-dump`/`wpctl` script), ARM, Flatpak, JACK — system sink fits poorly in sandbox.
- 2026-10-16: `FxController.cpp:1504` driver check → “PipeWire connected + sink created”, reuse `FxDeviceErrorMessage` — same UX path.
- 2026-10-16: File refs `FxController.cpp:841/:1504` etc. from earlier plans verified to exist on `d8e7a23`; if missing on other rev, grep equivalent and note here.
