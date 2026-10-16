/*
FxSound Linux port — linux/sysinfo_posix.h (Phase 8)
SysInfo Linux behavior:
  "Audio service OK" -> "PipeWire connected" (pwConnectedAndSinkReady).
  isRemoteSession() -> false. arch via SystemStats/cpuArch().
  enumAudioOutputs via pwListSinks (PipeWire enumerator), not MMDeviceEnumerator.
  canSupportHotkeys() -> false in v1 (Phase 9 KDE shortcuts call CLI).
Untested.
*/
#pragma once
#include <string>
#include <vector>

namespace fxsound_linux {
bool pipewireConnected();
std::vector<std::string> enumOutputs();  // friendly names, excludes fxsound sink
}  // namespace fxsound_linux
