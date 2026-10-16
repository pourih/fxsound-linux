# LINUX_AGENT_PROMPT — Fedora 44 KDE agent handoff

You run on a Fedora 44 KDE machine with hardware audio. The `linux` branch code is UNVERIFIED (written without Linux). Your job: install deps, build, fix errors, verify audio through PipeWire.

## 0. Ground rules (from final plan)
- Replace audio backend, not app. Keep `AudioPassthru`/`DfxDsp` public APIs + JUCE GUI; reimplement only Windows layers.
- Keep Windows building: `#if JUCE_WINDOWS` or small interfaces; new `*_posix`/`*_pw` siblings only. Never edit `.jucer/.vcxproj/.sln`.
- If a "Done when" fails twice: write to `docs/linux-port/BLOCKERS.md` and stop. Don't silently work around.
- Log every decision in `docs/linux-port/DECISIONS.md` (one-line reason).
- Out of scope v1: `Installer/`, `fxmcp`, `fxdiag` (use `pw-dump`/`wpctl` script), ARM, Flatpak, JACK.
- Wayland/licensing claims: verify, don't assume. Check `echo $XDG_SESSION_TYPE`.

## 1. Build loop
1. Follow `BUILD.md` top to bottom (dnf → cmake → ctest). Pin `third_party/JUCE` to exact 6.1.6 commit; record hash in DECISIONS.md. If GUI needs >~10 GCC 16 patches (see `patches/`), switch to JUCE 7.0.12 + Windows CMake CI proof per Phase 2 rule.
2. Fix `compat/` include-path issues first (scoped to DfxDsp/ptutil/AudioPassthruCore only; never PipeWire). No force-include of `win_compat.h`. Never define min/max/near/far/VOID.
3. Get `FxSound` GUI launching on stubs: expect 12 presets listed, settings persist. Send back log + screenshot. `AudioPassthruStub` fakes one device; real audio comes later.

## 2. Measurements before backend (Phase 3+4)
- Run `./tools/spike/pw_spike.sh`; fill `docs/linux-port/SPIKE_RESULTS.md` (every experiment + `VOLUME_MODE` A/B + `SINK_STRATEGY` native/persistent-dropin). Return `spike_out/` + console log.
- On Windows (or ask owner): build `tools/dsp_cli/dsp_cli.cpp`, generate `tests/golden/` refs for 12 presets + effects min/mid/max (see `docs/linux-port/GOLDEN.md`), commit inputs/refs/manifest. Self-run `compare_golden.py` must pass.

## 3. DSP validation (Phase 6)
- `compare_golden.py` within 1 LSB (report bit-exact %), BonusPresets load, ASan/UBSan clean, `param_effect.py` passes every effect. `RT_AUDIT.md` currently NOT_RT_SAFE → keep Phase 7 worker mode unless you hoist `processTimer` + lock-free setters and re-audit to RT_SAFE.
- Flags: `-O2 -ffp-contract=off -fno-strict-aliasing -fwrapv`. F32↔S16 via `linux/float_convert.h` (clamp, no attenuation). Do fixed-width `long`/`wchar_t` pass after ASan.

## 4. PipeWire backend (Phase 7) — verify in order, return pw-top/pw-dump/verbose log each
1. Passthrough DSP-off clean. 2. DSP-on obvious EQ audible. 3. Output switch, no feedback (never target fxsound sink). 4. USB/BT unplug/replug recovers; silence + wait if device gone (dont-reconnect/dont-fallback). 5. Clean quit restores `~/.local/state/fxsound/previous_sink` only if still fxsound; kill -9 leaves hardware audio. 6. Volume 100/50/10% consistent with VOLUME_MODE. 7. A/V sync acceptable (SPA_PARAM_ProcessLatency + click test at quanta 256/1024/2048).
- Startup: sink → registry → streams → default; retry PipeWire 30s. F32 48k/2ch, node.group/link-group, pre-allocate 8192, node.latency hint, drift telemetry every 60s, drop/repeat quantum rules. `pwConnectedAndSinkReady()` replaces `FxController.cpp:1504` check.

## 5. App + desktop + packaging (Phases 8-10)
- Apply `docs/linux-port/APP_LAYER.md` guards (crash→backtrace, `--status` stdout, XDG preset paths, message-window no-ops, SysInfo PipeWire, icons from BinaryData). Confirm: `fxsound --status` sane, second launch forwards + exits, presets in `~/.local/share/fxsound/presets/`, no `windows.h` from Linux FxSound build.
- `docs/linux-port/DESKTOP.md`: autostart minimized + routed, KDE shortcuts → CLI, tray SNI only if Plasma menu works (else ship no tray). Portal spike via `tools/spike/portal_shortcut_spike.sh`.
- `rpmbuild -ba packaging/fxsound.spec`; install on clean VM; `/usr/bin/fxsound` loads `/usr/share` presets. License AGPL-3.0-or-later.

## 6. Soak + report (Phase 11)
- `./tools/soak.sh` 8h: no glitches, no xruns, ≤1 drift correction/hour. Complete `docs/linux-port/SOAK.md` checklist (8 items). Exceptions → BLOCKERS.md with decision.

Return: build log, `spike_out/`, `soak_out/`, screenshots, updated DECISIONS/BLOCKERS/SPIKE_RESULTS/SOAK, and exact JUCE commit used.
