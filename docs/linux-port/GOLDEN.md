# Golden masters (Phase 4) [USER on Windows]

`dsp/Win32Main/` is a GUI test harness, not a CLI — `tools/dsp_cli/dsp_cli.cpp` is the new portable runner.

## Windows run (on Windows box with VS2022)

1. `python3 tools/golden/gen_inputs.py --dir tests/golden/inputs/` + add `music_excerpt.wav` (10s stereo 48k).
2. Build `dsp_cli` from `tools/dsp_cli/dsp_cli.cpp` + `dsp/` sources (link same as `DfxDsp.vcxproj`, no JUCE).
3. For each of 12 `bin/x64/Factsoft/*.fac` × each input in `manifest.json`, plus each effect at min/mid/max (`--preset=` + `--effect bass=X,ambience=Y,…`):
   `dsp_cli --in inputs/<in> --out refs/<out> --preset <fac> [...]`
4. Fill `tests/golden/manifest.json` (all cases), commit `inputs/`, `refs/`, `manifest.json`.
5. `python3 tests/compare_golden.py --manifest tests/golden/manifest.json --out-dir tests/golden/refs/` must PASS against itself (self-check).

Then Linux Phase 6 runs same `dsp_cli` (CMake) and compares within 1 LSB.

Untested here — Windows run pending.
