/*
FxSound Linux port — linux/message_window_posix.h (Phase 8)
Hoists FxController hidden message-window case bodies (WM_HOTKEY, PBT_APM*,
WTS*) into methods; Linux gets no-op bodies. Suspend/resume: no logind watcher
unless Phase 11 shows streams don't recover on their own.
Single-instance: moreThanOneInstanceAllowed()=false + anotherInstanceStarted()
  already forwards --power/preset/raise via JUCE on Linux; never create second sink.
Untested.
*/
#pragma once

namespace fxsound_linux {
inline void onHotkey(int) {}
inline void onPowerSuspend() {}
inline void onPowerResume() {}
inline void onSessionChange(int) {}
}  // namespace fxsound_linux
