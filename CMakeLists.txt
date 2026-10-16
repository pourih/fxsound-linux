# Linux port — CMake skeleton (Phase 5, GUI on stubs). Untested on Linux.
# Windows .jucer/.vcxproj/.sln are untouched. This file is Linux-only.
cmake_minimum_required(VERSION 3.22)
project(FxSoundLinux VERSION 1.2.15 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FXSOUND_LINUX_WERROR "Warnings-as-errors for new Linux files" ON)
if(FXSOUND_LINUX_WERROR)
  # Applied per-target below to NEW Linux files only (linux/, compat/, tools/).
  # Shared legacy sources keep default warnings to avoid Windows-parity breakage.
endif()

# ---- Phase 2 trivial target (kept so CI stays green) ----
add_library(fxsound_linux_trivial STATIC tools/ci/trivial.cpp)
target_include_directories(fxsound_linux_trivial PUBLIC tools/ci)

# ---- DfxDsp (static, JUCE-free) ----
# Core DSP + ptutil, excluding Win32-only harness and WinMM file-play path.
file(GLOB DFXDSP_CORE
  dsp/DfxDsp.cpp dsp/DfxDspPrivate.cpp dsp/DfxDspPreset.cpp dsp/DfxDspEq.cpp dsp/DfxDspRegistry.cpp
  dsp/ptutil/VALS/*.cpp dsp/ptutil/PRELST/*.cpp dsp/ptutil/dfxp/*.cpp
  dsp/ptutil/dfxSharedUtil/*.cpp dsp/ptutil/COM/*.cpp dsp/ptutil/MTH/*.cpp
  dsp/ptutil/MRY/*.cpp dsp/ptutil/pstr/*.cpp dsp/ptutil/ptime/*.cpp
  dsp/ptutil/SLOUT/*.cpp dsp/ptutil/realSample/*.cpp dsp/ptutil/Qnt/*.cpp
  dsp/ptutil/SOS/*.cpp dsp/ptechDsp/*.cpp dsp/ptechDsp/*/*.cpp dsp/ptComSftDfx/*.cpp dsp/ptComSftDfx/*/*.cpp
)
# Exclude legacy WinMM WAV file-play (needs mmsystem.h real impl) — v1 out of scope.
# PWAV headers still parse via compat/mmsystem.h stub, but no PWAV .cpp on Linux.
file(GLOB DFXDSP_PWAV dsp/ptutil/PWAV/*.cpp)
list(REMOVE_ITEM DFXDSP_CORE ${DFXDSP_PWAV})
# Note: glob may miss new subdirs; Linux agent: verify `git status` after build.

add_library(DfxDsp STATIC ${DFXDSP_CORE})
target_include_directories(DfxDsp PUBLIC
  dsp/include dsp audiopassthru/include dsp/ptutil/include
  dsp/ptutil/VALS dsp/ptutil/PRELST dsp/ptutil/PWAV dsp/ptutil/dfxp
)
# compat/ ONLY for DfxDsp/ptutil (never PipeWire). No force-include.
target_include_directories(DfxDsp PRIVATE compat)
target_compile_definitions(DfxDsp PRIVATE JUCE_MODAL_LOOPS_PERMITTED)
target_compile_options(DfxDsp PRIVATE -O2 -ffp-contract=off -fno-strict-aliasing -fwrapv)
# No JUCE, no crypt32/Wtsapi32/dbghelp on Linux.

# ---- AudioPassthruCore (static, no WASAPI, no PipeWire) ----
file(GLOB AP_CORE
  audiopassthru/src/AudioPassthru/*.cpp
  audiopassthru/src/MRY/*.cpp audiopassthru/src/MTH/*.cpp
  audiopassthru/src/pstr/*.cpp audiopassthru/src/ptime/*.cpp
  audiopassthru/src/SLOUT/*.cpp
)
# Excluded on Linux: src/sndDevices/* (WASAPI), src/reg/* (Win32 registry),
# src/FILE/fileWin32Handle.cpp (CreateFile), src/operatingSystem/* (SCM/WMP).
add_library(AudioPassthruCore STATIC ${AP_CORE}
  linux/registry_posix.cpp linux/dsp_registry_posix.cpp linux/os_posix.cpp
)
target_include_directories(AudioPassthruCore PUBLIC
  audiopassthru/include dsp/include linux
)
target_include_directories(AudioPassthruCore PRIVATE compat)
target_link_libraries(AudioPassthruCore PUBLIC DfxDsp)

# ---- AudioPassthruPW (PipeWire backend, Phase 7 real impl; Phase 5 stub dep) ----
add_library(AudioPassthruPW STATIC
  linux/AudioPassthruStub.cpp
  linux/sndDevices_pw.cpp
)
target_include_directories(AudioPassthruPW PUBLIC linux audiopassthru/include dsp/include)
# NOTE: no compat/ here by design — compat shims must never reach PipeWire code.
find_package(PkgConfig QUIET)
if(PkgConfig)
  pkg_check_modules(PIPEWIRE QUIET libpipewire-0.3)
  if(PIPEWIRE_FOUND)
    target_include_directories(AudioPassthruPW PRIVATE ${PIPEWIRE_INCLUDE_DIRS})
  endif()
endif()

# New Linux files get warnings-as-errors (shared legacy sources do not).
foreach(t fxsound_linux_trivial AudioPassthruCore AudioPassthruPW)
  if(FXSOUND_LINUX_WERROR)
    target_compile_options(${t} PRIVATE -Wall -Wextra -Werror)
  endif()
endforeach()
target_compile_options(DfxDsp PRIVATE -Wall -Wextra)
# (No -Werror on DfxDsp legacy shared code — would break Windows parity.)

# ---- dsp_cli (Phase 4/6 offline runner) ----
add_executable(dsp_cli tools/dsp_cli/dsp_cli.cpp)
target_include_directories(dsp_cli PRIVATE dsp/include)
target_link_libraries(dsp_cli PRIVATE DfxDsp)

# ---- FxSound GUI (JUCE, optional until submodule pinned) ----
# Expected: third_party/JUCE submodule at exact 6.1.6 commit (see DECISIONS.md).
# If absent, GUI target is skipped so CI trivial still passes; Linux agent must init it.
if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/JUCE/CMakeLists.txt)
  add_subdirectory(third_party/JUCE)
  file(GLOB FX_GUI
    fxsound/Source/Main.cpp fxsound/Source/GUI/*.cpp fxsound/Source/Utils/Settings/*.cpp
    fxsound/Source/Utils/SysInfo/*.cpp
  )
  # Exclude Windows-only tray/message-window impl from Linux GUI build; Linux replacements in Phase 8/9.
  list(FILTER FX_GUI EXCLUDE REGEX ".*FxSystemTrayView\\.cpp$")
  juce_add_gui_app(FxSound PRODUCT_NAME "FxSound")
  target_sources(FxSound PRIVATE ${FX_GUI} linux/AudioPassthruStub.cpp linux/registry_posix.cpp linux/os_posix.cpp)
  target_include_directories(FxSound PRIVATE
    fxsound/Source fxsound/Source/GUI audiopassthru/include dsp/include linux
    Resources/Strings
  )
  target_compile_definitions(FxSound PRIVATE
    JUCE_MODAL_LOOPS_PERMITTED=1 JUCE_WEB_BROWSER=0 JUCE_USE_CURL=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1 JUCE_DISPLAY_SPLASH_SCREEN=0
  )
  target_link_libraries(FxSound PRIVATE
    juce::juce_gui_extra juce::juce_gui_basics juce::juce_graphics
    juce::juce_data_structures juce::juce_events juce::juce_core juce::juce_cryptography
    AudioPassthruCore AudioPassthruPW DfxDsp
  )
  # No juce_opengl (unused per RECON §6). No crypt32/Wtsapi32/dbghelp.
  juce_generate_juce_header(FxSound)
else()
  message(WARNING "third_party/JUCE missing — FxSound GUI skipped (run git submodule add + checkout 6.1.6). Trivial + DSP targets still build.")
endif()

enable_testing()
add_test(NAME trivial_build COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fxsound_linux_trivial)
add_test(NAME dsp_cli_help COMMAND dsp_cli)
set_tests_properties(dsp_cli_help PROPERTIES WILL_FAIL TRUE)
