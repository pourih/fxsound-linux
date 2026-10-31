<p align="center">
  <img src="fxsound/Images/logo-red.svg" alt="FxSound Linux Logo" width="160">
</p>

# FxSound for Linux (`fxsound-linux`)

A native, high-performance Linux port of the renowned [FxSound](https://www.fxsound.com) audio enhancer. This port completely replaces legacy Windows kernel drivers (`fxvad.sys`) with an ultra-low-latency userspace PipeWire virtual sink, lock-free circular audio buffers, bit-exact DSP effect pipelines, and modern desktop integration (StatusNotifierItem tray, D-Bus menu, and XDG global shortcuts).

If you want studio-quality volume boost, dynamic range compression, bass enhancement, and parametric equalization on Linux without touching kernel drivers or complex equalizer plugins, FxSound runs out-of-the-box on modern Wayland and X11 desktops.

---

## Deep-Dive Guides

If you want to understand how FxSound works or explore how this Linux port was engineered, check out our companion guides written in plain English:

- **[How FxSound Works: Under the Hood](docs/HOW_FXSOUND_WORKS.md):** A beginner-friendly walkthrough explaining the "middleman" virtual speaker trick, how each of the 5 sound effect sliders reshapes your audio, and how presets work.
- **[How the Linux Port Differs from Windows](docs/LINUX_PORT_DIFFERENCES.md):** An architectural overview explaining our "keep the brain, swap the plumbing" philosophy, the 64-bit number fix, and step-by-step comparisons of how Windows parts were adapted for Linux.

---

## Architecture & Audio Pipeline

Unlike the original Windows implementation which required a proprietary kernel-mode virtual audio driver, `fxsound-linux` establishes a zero-privilege, userspace audio graph using PipeWire's native C API (`libpipewire-0.3`):

```text
 ┌────────────────────────────────────────────────────────┐
 │   Desktop Applications (Spotify, Firefox, Games, MPV)  │
 └───────────────────────────┬────────────────────────────┘
                             │  Audio Output (PCM F32)
                             ▼
 ┌────────────────────────────────────────────────────────┐
 │     Virtual Sink: "FxSound Audio Enhancer" (Default)   │
 │            (support.null-audio-sink / adapter)         │
 └───────────────────────────┬────────────────────────────┘
                             │  pw-capture (48kHz Stereo)
                             ▼
 ┌────────────────────────────────────────────────────────┐
 │             Lock-Free SPSC Ring Buffer                 │
 └───────────────────────────┬────────────────────────────┘
                             │
                             ▼
 ┌────────────────────────────────────────────────────────┐
 │              DFX DSP Audio Engine Worker               │
 │  • Clarity & High-Frequency Synthesizer                │
 │  • Ambience & Spatial Reverb                           │
 │  • Surround Sound Stereo Field Widening                │
 │  • Dynamic Boost Peak Limiter & Auto-Leveler           │
 │  • Bass Boost Resonant Sub-harmonic Shaper             │
 │  • 9-Band / 10-Band Parametric Equalizer               │
 └───────────────────────────┬────────────────────────────┘
                             │
                             ▼
 ┌────────────────────────────────────────────────────────┐
 │             Lock-Free SPSC Ring Buffer                 │
 └───────────────────────────┬────────────────────────────┘
                             │  pw-playback (Direct Target)
                             ▼
 ┌────────────────────────────────────────────────────────┐
 │      Physical Hardware Sink (Speakers, DAC, Headset)   │
 └────────────────────────────────────────────────────────┘
```

### Key Architectural Highlights
- **Zero Kernel Modules:** No custom out-of-tree kernel modules or root daemons. Runs entirely within the unprivileged user session.
- **Bi-directional Volume Mirroring:** Volume adjustments made via hardware multimedia keys, OS mixer sliders, DAC knobs, or the FxSound GUI automatically synchronize both ways with active feedback-loop prevention.
- **Lock-Free SPSC Audio Queues:** Latency-critical PipeWire audio callbacks communicate with the DSP worker thread via memory-aligned circular buffers with contiguous memory copies to guarantee zero buffer xruns.
- **StatusNotifierItem (SNI) Tray Daemon:** Closing the application window hides the GUI into the system tray rather than killing audio playback, maintaining continuous audio enhancement in the background.

---

## Features & Windows Parity Matrix

| Capability | Windows Original | Linux Port (`fxsound-linux`) | Status |
| :--- | :---: | :---: | :---: |
| **Clarity DSP** | Yes | Yes (LP64 64-bit alignment verified) | Built-in |
| **Ambience & Surround** | Yes | Yes (full psychoacoustic reverb & stereo field) | Built-in |
| **Dynamic Boost & Limiter** | Yes | Yes (transparent peak volume maximization) | Built-in |
| **Bass Boost** | Yes | Yes (dynamic harmonic bass synthesis) | Built-in |
| **9-Band Parametric EQ** | Yes | Yes (real-time frequency response manipulation) | Built-in |
| **Audio Routing Engine** | Win32 WASAPI + `fxvad.sys` | Native PipeWire 0.3 Userspace Graph | Built-in |
| **Volume Synchronization** | OS-level | Bi-directional (Virtual Sink ⇄ Hardware DAC) | Built-in |
| **System Tray Integration** | Win32 Taskbar Shell | StatusNotifierItem (SNI) + DBusMenu | Built-in |
| **Global Shortcuts** | Win32 `RegisterHotKey` | XDG Desktop Portal GlobalShortcuts & D-Bus | Built-in |
| **IPC & CLI Control** | Win32 Window Messages | UNIX Domain Socket (`$XDG_RUNTIME_DIR/fxsound-cli.sock`) | Built-in |
| **Headless Service** | Windows Service | Systemd User Unit (`fxsound.service`) | Built-in |

---

## Project Structure

The repository maintains a clean, modular structure separating legacy DSP algorithms, Linux platform abstractions, and UI code:

```text
fxsound-linux/
├── audiopassthru/     # Audio passthrough facade and format converter interfaces
├── compat/            # Minimal POSIX/Win32 header compatibility shims
├── dsp/               # Core Power Technology / DFX DSP audio algorithms (LP64-adapted)
│   ├── include/       # Fixed-width integer definitions (ptLong.h)
│   ├── ptechDsp/      # Aural, Maximizer, Delay, Lexicon, and Parametric EQ routines
│   └── ptutil/        # Coefficients, FIR/IIR filters, parameter blocks
├── fxsound/           # JUCE application front-end
│   ├── Source/        # Main window, pro & lite views, sliders, presets manager
│   ├── Images/        # Vector icons and visual branding assets
│   └── Fonts/         # Typography resources
├── linux/             # Native Linux backend implementations
│   ├── sndDevices_pw.cpp         # PipeWire null-sink manager & audio streams
│   ├── AudioPassthru_linux.cpp   # Passthrough facade bridge & DSP worker
│   ├── tray_sni.cpp              # StatusNotifierItem & DBusMenu implementation
│   ├── single_instance_posix.cpp # UNIX socket single-instance IPC
│   ├── registry_posix.cpp        # In-memory cached atomic settings store
│   └── crash_posix.cpp           # Async-signal-safe crash and backtrace logger
├── packaging/         # Systemd user service, desktop entry, and RPM spec
├── tools/             # dsp_cli offline testing harness and validation scripts
├── CMakeLists.txt     # Build configuration with Ninja & GCC/Clang
└── README.md          # Project documentation
```

---

## Prerequisites & Installation

### Required System Packages

Install the necessary development libraries and build tools for your Linux distribution:

- **Fedora / RHEL:**
  ```bash
  sudo dnf install -y gcc-c++ cmake ninja-build pipewire-devel alsa-lib-devel \
                      systemd-devel freetype-devel libcurl-devel fontconfig-devel \
                      libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel
  ```

- **Arch Linux / Manjaro:**
  ```bash
  sudo pacman -S --needed base-devel cmake ninja pipewire alsa-lib systemd \
                          freetype2 libcurl-gnutls fontconfig libx11 libxrandr \
                          libxinerama libxcursor
  ```

- **Ubuntu / Debian:**
  ```bash
  sudo apt-get update
  sudo apt-get install -y build-essential cmake ninja-build libpipewire-0.3-dev \
                          libasound2-dev libsystemd-dev libfreetype6-dev libcurl4-openssl-dev \
                          libfontconfig1-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev
  ```

---

## Building from Source

```bash
# Clone the repository
git clone https://github.com/pourih/fxsound-linux.git
cd fxsound-linux

# Configure with Release optimization
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Compile both GUI application and offline DSP CLI tool
cmake --build build

# Install binaries, desktop entry, icon, and systemd service (optional)
sudo cmake --install build
```

The compiled binaries will be located at:
- GUI Application: `build/FxSound_artefacts/Release/FxSound`
- Headless DSP CLI: `build/dsp_cli`

---

## Usage & Operation

### 1. Graphical User Interface (GUI)

Launch the visual interface from your application launcher or terminal:

```bash
# Launch directly
FxSound

# Launch minimized to tray on startup (ideal for autostart)
FxSound --run_minimized
```

From the visual interface, you can:
- Toggle enhancement on or off instantly with the large power button.
- Choose between curated factory presets (**General**, **Music**, **Voice**, **Gaming**, **Bass Boost**, **Streaming**).
- Fine-tune individual audio parameters (Clarity, Ambience, Surround Sound, Dynamic Boost, Bass Boost).
- Adjust the 9-band EQ to compensate for headphone or room acoustic responses.
- Minimize the window directly to the system tray.

### 2. Headless Daemon via Systemd (Background Service)

To run FxSound automatically in the background as part of your desktop audio stack:

```bash
# Enable and start the user service
systemctl --user enable --now fxsound.service

# Check service status and audio logs
systemctl --user status fxsound.service
journalctl --user -u fxsound.service -f
```

### 3. Command Line & IPC Automation

When FxSound is already running, secondary command-line invocations communicate with the primary instance over a UNIX domain socket (`$XDG_RUNTIME_DIR/fxsound-cli.sock`) or D-Bus:

```bash
# Toggle power state (on/off)
FxSound --power=toggle

# Load a specific preset
FxSound --preset="Music"
FxSound --preset="Gaming"

# Adjust individual effect parameters (0 to 10)
FxSound --set_effect=clarity:7,bass:8,dynamic:6

# Query current running status (JSON output)
FxSound --status

# Control via D-Bus session bus
gdbus call --session --dest org.fxsound.FxSound \
           --object-path /org/fxsound/FxSound \
           --method org.fxsound.FxSound.TogglePower
```

---

## Configuration & Preset Migration

### File Paths on Linux
FxSound adheres strictly to the XDG Base Directory specification:
- **Application Settings & Registry:** `~/.config/FxSound/linux_registry.ini`
- **User Presets:** `~/.config/FxSound/Presets/`
- **Crash & Diagnostic Logs:** `~/.config/FxSound/crash.log`
- **Runtime IPC Socket:** `$XDG_RUNTIME_DIR/fxsound-cli.sock`
- **Previous Sink State:** `~/.local/state/fxsound/previous_sink`

### Dual-Boot & Windows Preset Migration
If you dual-boot Windows and have custom presets created in the Windows version of FxSound:
1. Locate your presets folder on Windows:
   ```text
   C:\Users\<YourUsername>\AppData\Roaming\FxSound\Presets\
   ```
2. Copy the `.fac` preset files to your Linux user presets directory:
   ```bash
   mkdir -p ~/.config/FxSound/Presets
   cp /path/to/windows/Presets/*.fac ~/.config/FxSound/Presets/
   ```
3. Restart or reload FxSound; your custom EQ curves and effect levels will appear immediately.

---

## Troubleshooting & Latency Tuning

- **Latency Optimization:**
  To reduce buffer latency for pro-audio or competitive gaming, launch FxSound with the PipeWire quantum override:
  ```bash
  PIPEWIRE_LATENCY="512/48000" FxSound
  ```
- **Checking PipeWire Graph Routing:**
  Inspect active audio links between apps and the FxSound virtual sink:
  ```bash
  pw-top
  # or visual patchbays:
  helvum
  qpwgraph
  ```
- **Restoring Default Sound Device:**
  If you exit FxSound, it automatically restores your previous hardware sink. If the app was abnormally killed, you can restore your physical sink manually with:
  ```bash
  wpctl status
  wpctl set-default <device-id>
  ```

---

## Dual-Boot & Cross-Platform Git Sync

This repository includes `.gitattributes` to normalize line endings (`LF`) across Windows and Linux, preventing unstaged newline diffs when switching between operating systems.

- **On Windows:** Build using the Visual Studio solution (`fxsound/Project/FxSound.sln`).
- **On Linux:** Build using CMake and Ninja (`cmake --build build`).

---

## License & Acknowledgments

- **License:** GNU Affero General Public License v3.0 ([AGPL-3.0](LICENSE)).
- **Original Windows Application:** Developed and maintained by the [FxSound Community](https://github.com/fxsound2/fxsound-app).
- **JUCE Framework:** Licensed under AGPL-3.0 by [JUCE](https://juce.com).
- **PipeWire:** Multimedia graph framework licensed under MIT by [PipeWire](https://pipewire.org).
