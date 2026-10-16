/*
FxSound Linux port — linux/AudioPassthruPW.cpp (Phase 7)
Inherits stub interface; real start/stop delegate to sndDevices_pw.
Drift: ring fill counters, drop 1 quantum if >4 quanta, repeat last if empty.
Every correction counted, logged every 60s when verbose.
Untested.
*/
#include "AudioPassthruPW.h"
#include "sndDevices_pw.h"

namespace fxsound_linux {
AudioPassthruPW::AudioPassthruPW() = default;
bool AudioPassthruPW::startPw() {
  pwSetVerbose(true);
  return pwStartup("", 60);
}
void AudioPassthruPW::stopPw() { pwShutdown(""); }
}  // namespace fxsound_linux
