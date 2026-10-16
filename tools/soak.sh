#!/usr/bin/env bash
# soak.sh — 8h drift/xrun soak (Phase 11) [USER]. Untested here.
# Plays long test file through app, snapshots pw-top every 5min,
# greps journalctl --user for xruns. App verbose log records drift counters.
# Usage: ./tools/soak.sh <test.wav> [hours=8]
set -u
WAV="${1:-tests/golden/inputs/pink_10s.wav}"
HOURS="${2:-8}"
OUT="soak_out"; mkdir -p "$OUT"
echo "soak $HOURS h from $WAV — $(date)" | tee "$OUT/soak.log"
echo "pw-top snapshots every 5min to $OUT/pwtop_*.txt; journal xruns to $OUT/xruns.txt"
END=$((SECONDS + HOURS * 3600)); N=0
# loop playback to fxsound sink in background (adjust target to real default if testing passthrough)
pw-play --target=fxsound_audio_enhancer --loop=-1 "$WAV" >"$OUT/play.log" 2>&1 &
PLAYPID=$!
trap 'kill $PLAYPID 2>/dev/null; exit' INT TERM
while [ $SECONDS -lt $END ]; do
  N=$((N+1))
  pw-top -b -n 1 > "$OUT/pwtop_$(printf %04d $N).txt" 2>&1 || true
  journalctl --user --since "5 minutes ago" 2>/dev/null | grep -i -E "xrun|underrun|overrun|error" >> "$OUT/xruns.txt" || true
  echo "tick $N $(date) — drift: see app verbose log (underruns/overruns/corrections)" | tee -a "$OUT/soak.log"
  sleep 300
done
kill $PLAYPID 2>/dev/null || true
echo "done — attach $OUT/ + app verbose log. Pass: no glitches, no xruns, <=1 drift correction/hour." | tee -a "$OUT/soak.log"
