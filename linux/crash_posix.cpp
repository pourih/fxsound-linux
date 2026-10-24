/*
FxSound Linux port — linux/crash_posix.cpp (Phase 8)
POSIX replacement for Main.cpp MiniDump/StackWalk64 crash capture.
SIGSEGV/SIGABRT/SIGFPE/SIGILL/SIGBUS handlers append a backtrace to
~/.config/FxSound/crash.log (same location class as the Windows fxsound.dmp).
JUCE-free so it can be linked from any target. Windows never compiles this.
*/
#include "crash_posix.h"
#if !defined(_WIN32)
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

namespace fxsound_linux {
namespace {

std::string crashLogPath() {
  const char* xdg = std::getenv("XDG_CONFIG_HOME");
  std::string base = (xdg && *xdg) ? std::string(xdg)
                                   : std::string(std::getenv("HOME") ? std::getenv("HOME") : "/tmp") + "/.config";
  return base + "/FxSound/crash.log";
}

void writeBacktrace(FILE* f, void* const* frames, int n) {
  char** syms = backtrace_symbols(frames, static_cast<size_t>(n));
  for (int i = 0; i < n; i++) fprintf(f, "%s\n", syms ? syms[i] : "?");
  std::free(syms);
}

void crashHandler(int sig) {
  // async-signal-safe enough: open/write/close only, no malloc after frames
  void* bt[64];
  int n = backtrace(bt, 64);
  FILE* f = std::fopen(crashLogPath().c_str(), "a");
  if (f) {
    fprintf(f, "--- crash: signal %d (pid %d) ---\n", sig, static_cast<int>(getpid()));
    writeBacktrace(f, bt, n);
    fclose(f);
  }
  // Also dump to stderr for journal capture.
  writeBacktrace(stderr, bt, n);
  signal(sig, SIG_DFL);
  raise(sig);
}

}  // namespace

void installCrashHandler() {
  for (int s : {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS}) signal(s, crashHandler);
}

namespace {
volatile sig_atomic_t g_quit_requested = 0;
void quitSignalHandler(int) { g_quit_requested = 1; }
}  // namespace

void installQuitFlagSignals() {
  // JUCE traps SIGINT only; give TERM/HUP the same clean-quit chance so audio
  // state (default sink), the CLI socket and settings are all restored/saved.
  for (int s : {SIGTERM, SIGHUP}) {
    struct sigaction sa {};
    sa.sa_handler = quitSignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(s, &sa, nullptr);
  }
}

bool quitFlagConsumed() {
  if (g_quit_requested) {
    g_quit_requested = 0;
    return true;
  }
  return false;
}

void logBacktrace() {
  void* bt[64];
  int n = backtrace(bt, 64);
  FILE* f = std::fopen(crashLogPath().c_str(), "a");
  if (f) {
    fprintf(f, "--- stack ---\n");
    writeBacktrace(f, bt, n);
    fclose(f);
  }
}

bool writeStatusStdout(const char* json) {
  if (json) std::puts(json);
  return true;
}
}  // namespace fxsound_linux
#endif
