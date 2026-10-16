/*
FxSound Linux port — linux/AudioPassthruPW.h (Phase 7)
PipeWire-backed AudioPassthru behind same narrow interface as stub (Phase 5).
Uses pw_thread_loop; marshals events to JUCE message thread via callAsync.
Verbose logging from start. Untested.
*/
#pragma once
#include "AudioPassthruStub.h"
#include <string>

namespace fxsound_linux {
// Worker-mode driver: owns DSP instance pointer (opaque to avoid DfxDsp include
// in header), pulls capture ring -> processAudio -> playback ring.
class AudioPassthruPW : public AudioPassthruStub {
 public:
  AudioPassthruPW();
  // Same narrow interface as stub; overrides start/stop/setOutput for PipeWire.
  bool start() { return startPw(); }
  void stop() { stopPw(); }

 private:
  bool startPw();
  void stopPw();
  void* dsp_ = nullptr;  // DfxDsp* set via setDsp() in .cpp to keep header JUCE-free
 public:
  void setDsp(void* dsp) { dsp_ = dsp; }
};
}  // namespace fxsound_linux
