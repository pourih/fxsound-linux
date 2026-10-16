/*
FxSound Linux port — linux/AudioPassthruStub.h (Phase 5)
Narrow interface behind which AudioPassthruStub (fake list) and later
AudioPassthruPW (PipeWire, Phase 7) sit.
Derived from AudioPassthruPrivate.cpp + u_AudioPassthru.h:
  device list, set output, start/stop, buffer size, device callbacks.
Portable types only (no IMMDevice, no WCHAR[]). Untested.
*/
#pragma once
#include <functional>
#include <string>
#include <vector>

namespace fxsound_linux {
struct LinuxSoundDevice {
  std::string nodeName;       // persisted ID (PipeWire node.name); WASAPI pwszID equivalent
  std::string friendlyName;   // UI name (PipeWire node.description)
  std::string description;    // e.g. "Speakers"
  bool isDefault = false;
  bool isFxsound = false;
  int channels = 2;
};

class AudioPassthruStub {
 public:
  using DeviceCb = std::function<void(bool processing)>;
  AudioPassthruStub();
  std::vector<LinuxSoundDevice> getSoundDevices(bool activeOnly = true);
  bool setOutput(const std::string& nodeName);  // never targets fxsound sink (Phase 7 rule)
  bool start();
  void stop();
  bool setBufferLengthMs(int ms);  // maps to node.latency hint only (Phase 7)
  int bufferLengthMs() const { return bufferMs_; }
  void registerCallback(DeviceCb cb) { cb_ = std::move(cb); }

 private:
  int bufferMs_ = 60;
  std::string outputNode_;
  DeviceCb cb_;
};
}  // namespace fxsound_linux
