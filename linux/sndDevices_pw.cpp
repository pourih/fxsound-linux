/*
FxSound Linux port — linux/sndDevices_pw.cpp (Phase 5 placeholder, real impl Phase 7)
PipeWire backend behind sndDevices.h. Phase 5: compiles, returns no devices.
Phase 7 implements: null sink (fxsound_audio_enhancer), capture/playback
pw_stream F32 48k/2ch, pw_registry listener, default metadata, volume mirror,
drift telemetry, SPA_PARAM_ProcessLatency. Untested.
*/
#include <cstdio>

namespace fxsound_linux {
// Placeholder symbol so AudioPassthruPW always links in Phase 5 CI.
void sndDevicesPwPlaceholder() { std::puts("[pw] stub — Phase 7 implements backend"); }
}  // namespace fxsound_linux
