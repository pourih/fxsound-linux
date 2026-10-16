# SPIKE_RESULTS — PipeWire measurements (Phase 3) [USER]

Run `tools/spike/pw_spike.sh` on Fedora 44 KDE hardware, attach `spike_out/` + console log.
Fill every measured value. Untested here.

## Environment
- `pw-dump --version`:
- `wpctl --version`:
- `XDG_SESSION_TYPE` (verify Wayland/X11):
- Real sink node.name + description:
- Kernel / PipeWire / WirePlumber versions:

## 1. Sink ownership
- Native `support.null-audio-sink` + `object.linger=false` appears? (pass/fail + dump refs):
- Disappears after killing creator?:
- `pactl module-null-sink` appears/disappears?:
- Verdict: `object.linger=false` works as expected? YES/NO

## 2. Friendly name
- KDE applet shows `node.description="FxSound Audio Enhancer"`? YES/NO (+ screenshot):

## 3. Loopback
- `pw-loopback` monitor→real produces sound? YES/NO + exact commands used:

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
