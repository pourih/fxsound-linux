/*
FxSound Linux port — linux/AudioPassthruStub.cpp (Phase 5)
Fake list so GUI launches with no audio on Fedora (Phase 5 Done when).
Real backend in Phase 7 (sndDevices_pw.cpp + AudioPassthruPW).
Untested.
*/
#include "AudioPassthruStub.h"

namespace fxsound_linux {
AudioPassthruStub::AudioPassthruStub() : outputNode_("") {}

std::vector<LinuxSoundDevice> AudioPassthruStub::getSoundDevices(bool) {
  // 12 presets need no audio; return one fake hardware output so dropdown works.
  return {{"alsa_output_fake", "Fake Output (stub)", "Speakers", true, false, 2}};
}

bool AudioPassthruStub::setOutput(const std::string& nodeName) {
  if (nodeName == "fxsound_audio_enhancer") return false;  // never loop back
  outputNode_ = nodeName;
  return true;
}
bool AudioPassthruStub::start() { return true; }
void AudioPassthruStub::stop() {}
bool AudioPassthruStub::setBufferLengthMs(int ms) {
  if (ms < 10 || ms > 100) return false;
  bufferMs_ = ms;
  return true;
}
}  // namespace fxsound_linux
