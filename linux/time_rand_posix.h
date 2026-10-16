/*
FxSound Linux port — linux/time_rand_posix.h (Phase 5)
Sibling for Ptime + MthRand: std::chrono / <random>.
Untested.
*/
#pragma once
#include <chrono>
#include <random>

namespace fxsound_linux {
inline uint64_t nowMs() {
  return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}
inline uint32_t randU32(uint32_t lo, uint32_t hi) {
  thread_local std::mt19937 rng{std::random_device{}()};
  std::uniform_int_distribution<uint32_t> d(lo, hi);
  return d(rng);
}
}  // namespace fxsound_linux
