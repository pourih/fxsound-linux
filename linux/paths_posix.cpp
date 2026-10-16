/*
FxSound Linux port — linux/paths_posix.cpp (Phase 8). Untested.
*/
#include "paths_posix.h"
#include <cstdlib>
#include <filesystem>
namespace fs = std::filesystem;
namespace fxsound_linux {
static std::string xdg(const char* key, const char* fallback) {
  const char* v = std::getenv(key);
  if (v && *v) return v;
  const char* h = std::getenv("HOME");
  return std::string(h ? h : "/tmp") + fallback;
}
std::string factoryPresetDir(const std::string& exeDir) {
  if (fs::exists("/usr/share/fxsound/Factsoft")) return "/usr/share/fxsound/Factsoft";
  return (fs::path(exeDir) / "Factsoft").string();  // uninstalled fallback
}
std::string userPresetDir() { return xdg("XDG_DATA_HOME", "/.local/share") + "/fxsound/presets/"; }
std::string autoSaveDir() { return xdg("XDG_DATA_HOME", "/.local/share") + "/fxsound/AutoSave/"; }
std::string statusJsonPath() { return xdg("XDG_CONFIG_HOME", "/.config") + "/FxSound/status.json"; }
}  // namespace fxsound_linux
