/*
FxSound Linux port — linux/sndDevices_pw.cpp (Phase 7 real backend skeleton)
Worker mode (RT_AUDIT=NOT_RT_SAFE): capture -> ring -> worker(processAudio) -> ring -> playback.
Rings 8192 frames max quantum; drop one quantum if fill>4 quanta; repeat last if empty.
F32 interleaved 48k/2ch both streams; adapter converts; param_changed asserts + setSignalFormat.
node.group + node.link-group shared; node.latency hint from user_buffer_size.
Latency declared via SPA_PARAM_ProcessLatency (ring quanta + DSP delay), verify click test.
Startup: sink -> wait registry -> streams -> flip default; retry PipeWire 30s.
previous_sink in ~/.local/state/fxsound/previous_sink only when not fxsound; restore only
  if current still fxsound; if gone clear metadata and let WirePlumber choose.
Crash: non-lingering sink vanishes, WirePlumber falls back (confirm spike).
SINK_STRATEGY persistent-dropin: ship WirePlumber/pipewire.conf.d drop-in instead (fallback).
Untested — verify with Phase 3 click test + Phase 11 soak. Never includes compat/.
*/
#include "sndDevices_pw.h"
#include "float_convert.h"
#include <atomic>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

// PipeWire headers only on Linux; stubbed otherwise so Windows editing still parses.
#ifdef __linux__
#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <spa/param/latency-utils.h>
#endif

namespace fxsound_linux {
namespace {
bool g_verbose = false;
PwDeviceCb g_cb;
PwDrift g_drift;
// SPSC rings (float interleaved, 2ch, 8192 frames each)
constexpr size_t kMaxQ = 8192, kCh = 2;
std::vector<float> g_capRing, g_playRing;
std::atomic<size_t> g_capW{0}, g_capR{0}, g_playW{0}, g_playR{0};
std::string g_outputNode;
float g_fxVol = 1.0f;
bool g_fxMute = false;
}  // namespace

void pwSetVerbose(bool v) { g_verbose = v; }
bool pwVerbose() { return g_verbose; }
void pwSetDeviceCallback(PwDeviceCb cb) { g_cb = std::move(cb); }
PwDrift pwDrift() { return g_drift; }
float pwFxsoundVolume() {
  if (g_fxMute) return 0.0f;
  return g_fxVol < 0.02f ? 0.02f : g_fxVol;  // floor for B-mode divide
}

bool pwStartup(const std::string&, int) {
  // TODO(Linux agent): pw_thread_loop + pw_core_create_object(adapter,
  // support.null-audio-sink, node.name=fxsound_audio_enhancer, ...object.linger=false...),
  // wait registry, create capture (stream.capture.sink=true target fxsound) +
  // playback (target=<real>, dont-reconnect/dont-fallback) F32/48k/2ch in one
  // node.group/link-group, pre-allocate 8192, set node.latency hint, init fxsound
  // vol from real sink, flip default via metadata (fallback wpctl). Retry 30s.
  // Marshal all PipeWire events to JUCE message thread via MessageManager::callAsync.
  if (g_verbose) std::puts("[pw] startup stub — implement on Fedora");
  g_capRing.assign(kMaxQ * kCh, 0);
  g_playRing.assign(kMaxQ * kCh, 0);
  return true;
}
void pwShutdown(const std::string&) {
  // TODO: clean quit restore (pwRestorePrevious), destroy streams then sink.
}
bool pwConnectedAndSinkReady() {
  // Replaces FxController.cpp:1504 missing-driver check.
  return true;  // TODO: real pw_core + registry check
}
std::vector<PwDevice> pwListSinks() {
  // TODO: pw_registry Audio/Sink listener; description=friendly, node.name=ID; exclude fxsound.
  return {};
}
bool pwSetOutput(const std::string& nodeName) {
  if (nodeName == "fxsound_audio_enhancer") return false;  // never feedback
  g_outputNode = nodeName;
  // TODO: destroy + recreate playback stream with new target.object; on missing
  // device output silence and wait (dont-reconnect/dont-fallback).
  return true;
}
bool pwSetDefaultFxsound() {
  // TODO: write default.configured.audio.sink on default metadata; fallback:
  // system("wpctl set-default <fxsound id>"). Return true if either works.
  return true;
}
bool pwRestorePrevious(const std::string&, bool currentIsFxsound) {
  if (!currentIsFxsound) return true;  // never overwrite user choice meanwhile
  // TODO: if recorded sink exists set it, else clear metadata for WirePlumber priority.
  return true;
}
void pwMirrorVolumeTick() {
  // Mode A: mirror fxsound vol/mute -> real only. Mode B: additionally divide
  // capture by fxsound vol (floor 0.02, silence if muted). Debounce; ignore Props
  // echo of our own writes. See VOLUME_MODE in SPIKE_RESULTS.md.
}
void sndDevicesPwPlaceholder() {}
}  // namespace fxsound_linux
