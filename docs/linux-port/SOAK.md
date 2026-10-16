# SOAK + FINAL ACCEPTANCE (Phase 11) [USER]

Run `tools/soak.sh` 8h on Fedora 44 KDE hardware. Attach `soak_out/`,
`pw-top` snapshots, `journalctl --user` xrun grep, app verbose log.
Untested here. Exceptions → `BLOCKERS.md` with decision each.

## Checklist (all [USER], in order)
1. `--status` sane output. Result:
2. UI launches, 12 factory presets show, language switching works. Result:
3. Sink "FxSound Audio Enhancer" appears in KDE, becomes default, previous
   default restored on quit. Result (+ `pw-dump` refs):
4. Power off transparent; power on with extreme EQ audible. Result:
5. Golden WAV via `dsp_cli` on Fedora matches Windows reference
   (`compare_golden.py` within 1 LSB). Result:
6. USB DAC hotplug, Bluetooth, suspend/resume, app restart, SIGKILL crash all
   recover (audio back on hardware after kill -9). Result per item:
7. System volume at 100%/50%/10% keeps DSP consistent with VOLUME_MODE. Result:
8. Apps pinned to specific sink or JACK bypass effect (expected; Windows
   exclusive-mode same limit). Confirmed:

## Soak pass criteria
No audible glitches, no xruns in journal, zero drift corrections (or ≤1/hour,
each logged). `pwDrift()` underruns/overruns/corrections attached.
