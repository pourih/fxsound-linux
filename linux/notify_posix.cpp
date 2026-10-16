/*
FxSound Linux port — linux/notify_posix.cpp (Phase 9). Untested.
Prefers org.freedesktop.Notifications via gdbus/dbus-send fallback; keeps
FxNotification custom popup as fallback when notifications hidden=false.
Respects hide_notifications setting (caller checks before calling notify()).
*/
#include "notify_posix.h"
#include <cstdlib>
#include <string>

namespace fxsound_linux {
void notify(const std::string& title, const std::string& body) {
  // Best-effort, never blocks audio: shell out to gdbus (available on Plasma).
  std::string cmd =
      "gdbus call --session --dest org.freedesktop.Notifications "
      "--object-path /org/freedesktop/Notifications "
      "--method org.freedesktop.Notifications.Notify "
      "'FxSound' 0 '' '" + title + "' '" + body + "' '[]' '{}' 5000 "
      ">/dev/null 2>&1 &";
  (void)std::system(cmd.c_str());
}
}  // namespace fxsound_linux
