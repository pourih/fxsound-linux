/*
FxSound Linux port — linux/file_posix.h (Phase 5)
POSIX sibling for FILE/*, FileDate, FileGeneral, PWAV/* file I/O.
Originals use CreateFile/CreateFileW, _wfopen_s, WinMM mmio.
Linux uses std::filesystem + std::fstream (UTF-8 paths).
Untested on Linux.
*/
#pragma once
#include <filesystem>
#include <fstream>
#include <string>

namespace fxsound_linux {
// Join + normalize (replaces L"\\" + name + ".fac" patterns).
inline std::filesystem::path joinPreset(const std::filesystem::path& dir,
                                        const std::wstring& name) {
  std::filesystem::path p(dir);
  // wstring -> utf8 via filesystem path round-trip
  p /= std::filesystem::path(name);
  p.replace_extension(".fac");
  return p;
}
inline bool fileExists(const std::filesystem::path& p) {
  std::error_code ec;
  return std::filesystem::exists(p, ec);
}
}  // namespace fxsound_linux
