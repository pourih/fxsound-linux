/*
FxSound Linux port — linux/crash_posix.cpp (Phase 8). Untested.
Keep Windows building: all Win32 guarded by #if JUCE_WINDOWS in Main.cpp
(see APP_LAYER.md); this TU compiles on Linux only.
*/
#include "crash_posix.h"
#if !defined(_WIN32)
#include <cstdio>
#include <execinfo.h>
#include <cstdlib>
namespace fxsound_linux {
static void handler(void*) {
  void* bt[64];
  int n = backtrace(bt, 64);
  const char* home = std::getenv("HOME");
  std::string p = std::string(home ? home : "/tmp") + "/.config/FxSound/crash.log";
  FILE* f = fopen(p.c_str(), "a");
  if (f) {
    fprintf(f, "--- crash ---\n");
    backtrace_symbols_fd(bt, n, fileno(f));
    fclose(f);
  }
}
void installCrashHandler() {
  // JUCE: SystemStats::setApplicationCrashHandler(handler);
  // Included by FxSound target only (needs JuceHeader); kept header-only-safe here.
  (void)handler;
}
bool writeStatusStdout(const char* json) {
  if (json) std::puts(json);
  return true;
}
}  // namespace fxsound_linux
#endif
