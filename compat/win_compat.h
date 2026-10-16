/*
FxSound Linux port — compat/win_compat.h (Phase 5)
Narrow Windows shim for DfxDsp / ptutil / AudioPassthruCore ONLY.
Never force-included; never on PipeWire include path.
Uses <cstdint> types. Does NOT define min/max/near/far/VOID.
Untested on Linux.
*/
#pragma once
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <thread>
#include <chrono>

using DWORD = uint32_t;
using LONG = int32_t;
using BOOL = int32_t;
using BYTE = uint8_t;
using WORD = uint16_t;
using HANDLE = void*;
using HWND = void*;
using HINSTANCE = void*;
using HRESULT = int32_t;
// Neutralize MSVC declspec on GCC (dllexport/dllimport only; Phase 6).
// Does NOT define min/max/near/far/VOID per plan.
#ifndef __declspec
#define __declspec(x)
#endif
constexpr HRESULT S_OK = 0;
constexpr HRESULT E_FAIL = (HRESULT)0x80004005;
inline bool SUCCEEDED(HRESULT h) { return h >= 0; }
inline bool FAILED(HRESULT h) { return h < 0; }

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

inline void Sleep(DWORD ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// _stricmp / _strnicmp / _wcsicmp as inline functions (not macros)
inline int _stricmp(const char* a, const char* b) {
  for (; *a && *b; ++a, ++b) {
    int ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : (unsigned char)*a;
    int cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : (unsigned char)*b;
    if (ca != cb) return ca - cb;
  }
  return (unsigned char)*a - (unsigned char)*b;
}
inline int _strnicmp(const char* a, const char* b, size_t n) {
  for (size_t i = 0; i < n; ++i, ++a, ++b) {
    if (!*a && !*b) return 0;
    int ca = (*a >= 'A' && *a <= 'Z') ? *a + 32 : (unsigned char)*a;
    int cb = (*b >= 'A' && *b <= 'Z') ? *b + 32 : (unsigned char)*b;
    if (ca != cb) return ca - cb;
    if (!*a) break;
  }
  return 0;
}
inline int _wcsicmp(const wchar_t* a, const wchar_t* b) {
  for (; *a && *b; ++a, ++b) {
    wchar_t ca = (*a >= L'A' && *a <= L'Z') ? *a + 32 : *a;
    wchar_t cb = (*b >= L'A' && *b <= L'Z') ? *b + 32 : *b;
    if (ca != cb) return (int)(ca - cb);
  }
  return (int)(*a - *b);
}
inline int wcscpy_s(wchar_t* dst, size_t n, const wchar_t* src) {
  if (!dst || !src || n == 0) return 1;
  size_t i = 0;
  for (; i + 1 < n && src[i]; ++i) dst[i] = src[i];
  dst[i] = 0;
  return src[i] ? 1 : 0;
}
template <size_t N>
inline int wcscpy_s(wchar_t (&dst)[N], const wchar_t* src) {
  return wcscpy_s(dst, N, src);
}
