/*
FxSound Linux port — compat/mmsystem.h stub (Phase 5)
PWAV legacy WinMM path is not ported in v1. This stub lets PWAV headers
parse on Linux; PWAV runtime is excluded from Linux build (see CMake).
Untested.
*/
#pragma once
#include "win_compat.h"
using HMMIO = void*;
using HWAVEIN = void*;
using HWAVEOUT = void*;
using HPSTR = char*;
struct WAVEFORMATEX { WORD wFormatTag; WORD nChannels; DWORD nSamplesPerSec; DWORD nAvgBytesPerSec; WORD nBlockAlign; WORD wBitsPerSample; WORD cbSize; };
struct WAVEHDR { char* lpData; DWORD dwBufferLength; };
