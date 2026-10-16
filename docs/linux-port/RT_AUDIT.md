# RT_AUDIT — DfxDsp::processAudio realtime safety (Phase 6)

Verdict: **NOT_RT_SAFE** (as-written) → Phase 7 uses worker mode (rings).

## Path audited
- `dsp/DfxDsp.cpp:106-116` → `dsp/DfxDspPrivate.cpp:181-189`
  `processAudio()` calls `processTimer()` then `dfxpUniversalModifySamples()`.
- `dsp/DfxDspPrivate.cpp:157-179` `processTimer()`: checks `update_from_registry_`,
  calls `eqUpdateFromRegistry()` + `dfxpCommunicateAll()` when changed.
- `dsp/ptutil/dfxp/dfxpUniversal.cpp:89-...` chunks `>16384` (`DAW_MAX_BUFFER_SIZE`),
  no malloc in hot loop observed; `Comsftwr*.cpp:284-285,845-846` malloc only in
  init/setFormat path, not per-buffer. `swprintf` trace blocks are commented out
  (`dfxpUniversal.cpp:116-122`).

## Findings (conservative, untested on Linux)
- **File/registry in audio thread: YES (conditional).** `processTimer()` can hit
  VALS/registry (`eqUpdateFromRegistry`, `dfxpCommunicateAll`, `valsRead`) when
  `update_from_registry_` is set (preset/EQ change). Fast path when idle is two
  flag checks, but worst case blocks audio thread on file I/O.
- **Allocation: UNSURE in full dfxp tree.** No `malloc/calloc/new` found in
  `dfxpUniversalModifySamples` hot loop; init paths allocate (`Comsftwr`).
  Full-tree `new/malloc` in every effect (`Aural/Lex/Play/Maxi`) not exhaustively
  audited — assume possible until ASan/UBSan golden runs pass.
- **Locks: none found** in `processAudio` path by grep (`mutex|lock` only hits
  are comments/false positives). UNSURE — JUCE message-thread `applyConfig`
  vs audio thread parameter setters need atomics/command queue (Phase 7).
- **Logging: commented out** in hot path (`wcp_msg1`/`slout1` traces disabled).
  `DfxDspPrivate::processTimer` itself does no logging when idle.

## Mitigation (for RT_SAFE later, not applied in v1 to keep diff minimal)
- Hoist `processTimer()` out of `processAudio()` into `setSignalFormat` /
  `setNumBands` / UI-thread timer; make effect/EQ setters lock-free (atomics or
  SPSC command queue drained outside audio callback).
- Until then Phase 7 runs worker mode: capture → ring → worker (`processAudio`)
  → ring → playback, rings 8192 frames, drift telemetry, drop/repeat quantum
  on over/underflow. Inline F32 path from plan is deferred.

## Sample rates / block sizes (from RECON §8)
- Rates: device layer forces 44.1/48k; DSP consts 44100/48000 max; 88.2/96/192k UNSURE.
  Phase 7 requests F32 48k/2ch; `param_changed` asserts format, calls
  `setSignalFormat` on rate change.
- Blocks: variable `num_sets` per callback; DSP internally chunks >16384.
  `Reblocker` (linux/float_convert.h) available; max quantum 8192 per Phase 7.
- Delay: no end-to-end spec; internals 0.75ms look-ahead to 36ms taps — declare
  ring quanta + DSP delay via `SPA_PARAM_ProcessLatency`, verify with click test.
