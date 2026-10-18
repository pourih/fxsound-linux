# SPIKE_RESULTS — PipeWire measurements (Phase 3) [USER]

Run `tools/spike/pw_spike.sh` on Fedora 44 KDE hardware, attach `spike_out/` + console log.
Fill every measured value. Untested here.

## Environment
- `pw-dump --version`: n/a (no flag); `pipewire --version` → libpipewire 1.6.9 (compiled+linked)
- `wpctl --version`: n/a flagless; `wireplumber --version` → libwireplumber 0.5.18
- `XDG_SESSION_TYPE`: wayland (verified)
- Real sink node.name + description: `alsa_output.pci-0000_00_1f.3.analog-stereo` "Built-in Audio Analog Stereo" (device.api=alsa; note: currently muted per wpctl)
- Kernel / PipeWire / WirePlumber versions: 6.18.57-200.fc44.x86_64 / PipeWire 1.6.9 (pactl 17.0, pw-top present) / WirePlumber 0.5.18

## 1. Sink ownership
- Native `support.null-audio-sink` + `object.linger=false` appears? (pass/fail + dump refs):
  PASS after script fix — original `pw_spike.sh` used broken pw-cli quoting (multiline heredoc → per-line command errors in `pwcli_native.log`; node never created). Re-tested with fixed one-liner:
  `pw-cli create-node adapter '{ factory.name="support.null-audio-sink" node.name="fxsound_native_test" node.description="FxSound Native Test" media.class="Audio/Sink" audio.position="[ FL,FR ]" object.linger="false" node.pause-on-idle="false" }'`
  → node id 144 appears in pw-dump with media.class=Audio/Sink, linger=false. (Note: `pw-cli` non-interactive exits immediately, and a linger=false node is destroyed at creator exit — so the checker must keep the creator process alive, as any embedded PipeWire client in the app will.)
- Disappears after killing creator? YES — node gone from pw-dump within ~1.5s of SIGKILL of the pw-cli creator (linger=false semantics confirmed).
- `pactl module-null-sink` appears/disappears? YES — `dump_05_pactl.json` shows `fxsound_test` "FxSound_Test" (module id 536870916, `pactl_module.txt`); gone again in `dump_06_pactl_unloaded.json` after unload.
- Verdict: `object.linger=false` works as expected? YES (native path verified 2026-10-17; the png of dump refs: spike_out/dump_01..06, pwcli_native.log for the original script's quoting failure)

## 2. Friendly name
- KDE applet shows `node.description="FxSound Audio Enhancer"`? YES/NO (+ screenshot): MANUAL (user-side; needs KDE applet open while the app-created sink exists — `friendly_names.txt` confirms descriptions propagate to wpctl: real-sink entries show `name => description` pairs, so plumbing works on our end)

## 3. Loopback
- `pw-loopback` monitor→real produces sound? YES/NO + exact commands used:
  MANUAL (user-side listening check; command verified against installed `pw-loopback 1.6.9`, help captured in `pw-loopback_help.txt`):
  `pw-loopback --capture-props='target.object=<fxsink-node-id-or-name>.monitor audio.position=[ FL,FR ]' --playback-props='target.object=alsa_output.pci-0000_00_1f.3.analog-stereo audio.position=[ FL,FR ]'`
  then `pw-play --target <fxsink> spike_out/tone_1k_-20dbfs.wav` and confirm tone on speakers.

## 4. Monitor volume (VOLUME_MODE decision)
| fxsound vol | monitor.channel-volumes | monitor level vs -20dBFS ref | pre/post? |
|---|---|---|---|
| 100% | false | | |
| 50% | false | | |
| 10% | false | | |
| 100% | true | | |
| 50% | true | | |
| 10% | true | | |
- **VOLUME_MODE: A (pre-volume) / B (post-volume)** ← circle one, with reason.

## 5. Idle behavior
- Capture callback when no app plays: RUNNING / SUSPENDED / IDLE?
- Monitor silent or stopped?
- `node.pause-on-idle=false` effect:
- `session.suspend-timeout-seconds=0` effect:

## 6. Default switching
- After `wpctl set-default <fx>`, already-playing streams move?:
- Explicit-target streams stay?:

## 7. Crash behavior
- After SIGKILL creator, current default becomes?:
- Audio stays on hardware? YES/NO

## 8. Latency (round-trip ms)
| quantum | measured | notes |
|---|---|---|
| 256 | | |
| 1024 | | |
| 2048 | | |

## 9. Format
- `pw-top` negotiated format/rate/channels for sink + monitor:

## Decisions
- **VOLUME_MODE: ___** (A=monitor pre-volume → mirror fxsound→real only; B=post-volume → also divide capture by fxsound vol, floor 0.02, silence if muted)
- **SINK_STRATEGY: ___** (`native` if linger=false works; else `persistent-dropin` WirePlumber/pipewire.conf.d fallback — crash leaves silent default)
