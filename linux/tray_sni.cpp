/*
FxSound Linux port — linux/tray_sni.cpp (Phase 9 v1.1 stub). Untested.
v1 ships with no tray (return false). Full SNI + dbusmenu impl goes here.
Menu actions: power toggle, next/prev preset, open/close, quit — same IDs as
HK_CMD_ON_OFF/OPEN_CLOSE/NEXT_PRESET/PREVIOUS_PRESET/NEXT_OUTPUT.
Plasma renders menu; app only handles com.canonical.dbusmenu Event calls.
*/
#include "tray_sni.h"

namespace fxsound_linux {
bool traySniStart() { return false; }  // v1: no tray
void traySniStop() {}
void traySniSetStatus(bool, bool) {}
}  // namespace fxsound_linux
