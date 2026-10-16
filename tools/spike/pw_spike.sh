#!/usr/bin/env bash
# pw_spike.sh — PipeWire behavior spike for FxSound Linux port (Phase 3).
# Fedora 44 KDE, PipeWire + WirePlumber. Scripted, repeatable. Budget up to a day.
# Saves pw-dump before/after every step to spike_out/, prints pass/fail summary.
# Verifies flags on installed version. Does NOT require listening — uses meters/recordings.
# Usage: ./tools/spike/pw_spike.sh  (run on Fedora hardware; copy spike_out/ + console log back)
# Untested here — verify on hardware. If a step fails twice, log to docs/linux-port/BLOCKERS.md and stop.
set -u
OUT="spike_out"
mkdir -p "$OUT"
PASS=0; FAIL=0
log() { echo "[spike] $*"; }
pass() { echo "PASS: $*"; PASS=$((PASS+1)); }
fail() { echo "FAIL: $*"; FAIL=$((FAIL+1)); }
dump() { # $1 = tag
  pw-dump > "$OUT/dump_$1.json" 2>"$OUT/dump_$1.err" || echo "pw-dump failed for $1" >&2
}
need() {
  if ! command -v "$1" >/dev/null 2>&1; then fail "missing tool $1"; return 1; fi
  return 0
}
check_flags() {
  log "versions:"
  pw-dump --version 2>&1 | head -1 | tee "$OUT/versions.txt"
  wpctl --version 2>&1 | head -1 | tee -a "$OUT/versions.txt"
  pw-cli --version 2>&1 | head -1 | tee -a "$OUT/versions.txt" || true
  pactl --version 2>&1 | head -1 | tee -a "$OUT/versions.txt" || true
  pw-top --version 2>&1 | head -1 | tee -a "$OUT/versions.txt" || true
  pw-play --help 2>&1 | head -20 > "$OUT/pw-play_help.txt" || true
  pw-record --help 2>&1 | head -20 > "$OUT/pw-record_help.txt" || true
  pw-loopback --help 2>&1 | head -20 > "$OUT/pw-loopback_help.txt" || true
  echo "session type: ${XDG_SESSION_TYPE:-unknown}" | tee -a "$OUT/versions.txt"
}

for t in pw-dump wpctl pw-cli pw-top pw-play pw-record; do need "$t" || true; done
check_flags
dump "00_initial"
wpctl status > "$OUT/wpctl_initial.txt" 2>&1 || true
pw-top -b -n 1 > "$OUT/pwtop_initial.txt" 2>&1 || true

# --- 1. Sink ownership: native null sink with object.linger=false ---
log "1. native null sink ownership (object.linger=false)"
dump "01_before_native"
# pw-cli interactive session: create node then keep process alive in background
pw-cli -m <<'EOF' >"$OUT/pwcli_native.log" 2>&1 &
create-node adapter {
  factory.name=support.null-audio-sink
  object.linger=false
  media.class=Audio/Sink
  node.name=fxsound_audio_enhancer
  node.description="FxSound Audio Enhancer"
  audio.position=[ FL FR ]
  node.pause-on-idle=false
  session.suspend-timeout-seconds=0
}
EOF
NATIVE_PID=$!
sleep 3
dump "02_native_created"
if pw-dump | grep -q fxsound_audio_enhancer; then pass "native sink appears"; else fail "native sink appears"; fi
# kill creator -> sink should disappear (non-lingering)
kill "$NATIVE_PID" 2>/dev/null || true
sleep 2
dump "03_native_killed"
if pw-dump | grep -q fxsound_audio_enhancer; then fail "native sink disappears after kill (still present)"; else pass "native sink disappears after kill"; fi

# --- 1b. pactl module-null-sink lifetime ---
log "1b. pactl module-null-sink lifetime"
dump "04_before_pactl"
MODID=$(pactl load-module module-null-sink sink_name=fxsound_test sink_properties=device.description="FxSound_Test" 2> "$OUT/pactl_load.err" | tail -1) || MODID=""
echo "module id: $MODID" | tee "$OUT/pactl_module.txt"
sleep 2
dump "05_pactl_created"
if pw-dump | grep -q fxsound_test; then pass "pactl sink appears"; else fail "pactl sink appears"; fi
if [ -n "${MODID:-}" ]; then pactl unload-module "$MODID" || true; sleep 2; fi
dump "06_pactl_unloaded"
if pw-dump | grep -q fxsound_test; then fail "pactl sink disappears after unload"; else pass "pactl sink disappears after unload"; fi

# --- 2. Friendly name (manual KDE check) ---
log "2. friendly name: re-create native sink, check KDE applet shows 'FxSound Audio Enhancer'"
pw-cli -m <<'EOF' >"$OUT/pwcli_friendly.log" 2>&1 &
create-node adapter {
  factory.name=support.null-audio-sink
  object.linger=false
  media.class=Audio/Sink
  node.name=fxsound_audio_enhancer
  node.description="FxSound Audio Enhancer"
  audio.position=[ FL FR ]
}
EOF
FRIENDLY_PID=$!
sleep 3
dump "07_friendly"
pw-dump | python3 -c "import json,sys; d=json.load(sys.stdin); [print(n['info'].get('props',{}).get('node.name'), '=>', n['info'].get('props',{}).get('node.description')) for n in d if n.get('type','').endswith('Node')]" > "$OUT/friendly_names.txt" 2>&1 || true
cat "$OUT/friendly_names.txt" || true
echo "MANUAL: open KDE sound applet, confirm 'FxSound Audio Enhancer' listed. Record YES/NO in SPIKE_RESULTS.md" | tee -a "$OUT/friendly_names.txt"
kill "$FRIENDLY_PID" 2>/dev/null || true; sleep 1

# --- 3-5 need a live sink + tone. Generate -20dBFS 1kHz 10s tone ---
log "3-5. tone + loopback + monitor volume + idle"
if command -v sox >/dev/null 2>&1; then
  sox -n -r 48000 -c 2 "$OUT/tone_1k_-20dbfs.wav" synth 10 sine 1000 gain -20 || fail "sox tone"
else
  # fallback: python wav
  python3 - "$OUT/tone_1k_-20dbfs.wav" <<'PY' || fail "tone gen"
import wave, struct, math
sr=48000; dur=10; amp=0.1
w=wave.open(__import__('sys').argv[1],'wb'); w.setnchannels(2); w.setsampwidth(2); w.setframerate(sr)
for i in range(sr*dur):
  v=int(amp*32767*math.sin(2*math.pi*1000*i/sr))
  w.writeframes(struct.pack('<hh',v,v))
w.close()
PY
fi
ls -la "$OUT/tone_1k_-20dbfs.wav" && pass "tone generated" || fail "tone generated"

# Recreate fxsound sink for loopback tests
pw-cli -m <<'EOF' >"$OUT/pwcli_loop.log" 2>&1 &
create-node adapter {
  factory.name=support.null-audio-sink
  object.linger=false
  media.class=Audio/Sink
  node.name=fxsound_audio_enhancer
  node.description="FxSound Audio Enhancer"
  audio.position=[ FL FR ]
  node.pause-on-idle=false
  session.suspend-timeout-seconds=0
  monitor.channel-volumes=false
}
EOF
LOOP_PID=$!
sleep 3
REAL_SINK=$(wpctl status 2>/dev/null | grep -m1 -oP '^\s*\d+\.\s*\K.*(?=\s*\[vol)' || true)
echo "real sink guess: $REAL_SINK" | tee "$OUT/real_sink.txt"
dump "08_loop_ready"
echo "MANUAL loopback: run: pw-loopback --capture-props='target.object=fxsound_audio_enhancer' --playback-props='target.object=<real-sink-node.name>'  (find node.name via pw-dump). Play tone to fxsound sink, confirm sound on real device. Record in SPIKE_RESULTS.md." | tee "$OUT/loopback_manual.txt"
echo "MANUAL monitor volume: play tone at -20dBFS to fxsound sink; record monitor with: pw-record --target fxsound_audio_enhancer.monitor --format wav test_100.wav (100%), then wpctl set-volume <fx-id> 0.5 / 0.1 and record again. Repeat with monitor.channel-volumes=true sink. Note pre/post. See SPIKE_RESULTS.md table." | tee -a "$OUT/loopback_manual.txt"
echo "MANUAL idle: stop all playback 10s; check pw-top whether capture/monitor still RUNNING vs SUSPENDED/IDLE. Test node.pause-on-idle=false and session.suspend-timeout-seconds=0 variants." | tee -a "$OUT/loopback_manual.txt"

# --- 6. Default switching ---
log "6. default switching"
dump "09_before_default"
wpctl status > "$OUT/wpctl_before_default.txt" 2>&1 || true
echo "MANUAL: wpctl set-default <fxsound id>; start already-playing stream to old default BEFORE switch; check if it moves. Start stream with explicit target (pw-play --target <real>) and check it does NOT move. Record." | tee "$OUT/default_manual.txt"

# --- 7. Crash behavior ---
log "7. crash: SIGKILL creator, note new default"
echo "MANUAL: with fxsound sink as default, kill -9 $LOOP_PID (or creator PID); run wpctl status + pw-dump; note which sink becomes default. Record." | tee "$OUT/crash_manual.txt"

kill "$LOOP_PID" 2>/dev/null || true; sleep 1
dump "10_after_tests"

# --- 8. Latency (manual click) ---
cat > "$OUT/latency_manual.txt" <<'LTXT'
MANUAL latency: play click WAV through sink -> monitor -> loopback -> real sink.
Record loopback output + electrical/acoustic reference, measure round trip.
Repeat at quantum 256, 1024, 2048 (pw-metadata -n settings 0 clock.force-quantum <N>).
Record added delay in ms + frames. Used for SPA_PARAM_ProcessLatency (Phase 7).
LTXT
cat "$OUT/latency_manual.txt"

# --- 9. Format ---
log "9. format via pw-top"
pw-top -b -n 1 > "$OUT/pwtop_final.txt" 2>&1 || true
cat "$OUT/pwtop_final.txt" || true
dump "11_final"

echo ""
echo "=== SUMMARY ==="
echo "PASS=$PASS FAIL=$FAIL"
echo "Outputs in $OUT/. Copy spike_out/ + this log to docs/linux-port/ reporter."
echo "Fill docs/linux-port/SPIKE_RESULTS.md (VOLUME_MODE A/B, SINK_STRATEGY native/persistent-dropin)."
