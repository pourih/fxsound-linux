# Linux port — minimal CMake (Phase 2 trivial target, expanded in Phase 5)
# Windows .jucer/.vcxproj/.sln are untouched. This file is Linux-only.
cmake_minimum_required(VERSION 3.22)
project(FxSoundLinux VERSION 1.2.15 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warnings-as-errors applies to NEW Linux files only (see compat/, linux/, targets below).
# Shared legacy sources keep project warnings to avoid breaking Windows parity.
option(FXSOUND_LINUX_WERROR "Warnings-as-errors for new Linux files" ON)

if(FXSOUND_LINUX_WERROR)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror>)
endif()

# Phase 2 trivial target so CI has something to build + test.
# Phase 5 replaces this with DfxDsp / AudioPassthruCore / AudioPassthruPW / FxSound.
add_library(fxsound_linux_trivial STATIC
  tools/ci/trivial.cpp
)
target_include_directories(fxsound_linux_trivial PUBLIC tools/ci)

enable_testing()
add_test(NAME trivial_build COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fxsound_linux_trivial)
