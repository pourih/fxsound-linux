# How the Linux Port Differs from Windows FxSound

When porting a beloved Windows application like FxSound to Linux, many developers make the mistake of tearing everything apart and rewriting it from scratch. That approach often leads to subtle bugs, broken audio quality, and years of endless bug-hunting.

Our approach took the opposite route: **Keep the original brain, and only replace the plumbing.**

Here is an easy-to-understand breakdown of what stayed 100% original, what was adapted for Linux, and how the two versions compare.

---

## 1. The Core Philosophy: "Keep the Brain, Swap the Plumbing"

Think of FxSound as a luxury car. The engine under the hood is the audio algorithm math, and the steering wheel and dashboard are the user interface. The fuel lines and exhaust pipes are the operating system plumbing.

### What is 100% Original Code:
- **The Audio "Brain" (`dsp/` directory):** All the sound-enhancing math—the Clarity exciter, the Ambience reverb, the Surround widener, the Dynamic Boost compressor, the Bass enhancer, and the 9-band EQ—is the exact same C and C++ code running on Windows (`libDfxDsp.a`). Because we didn't touch this math, FxSound on Linux produces sound that is bit-for-bit identical to the Windows version.
- **The Visual "Dashboard" (`fxsound/Source/` using JUCE):** The user interface—the sliders, buttons, visual EQ curves, presets dropdown, and animations—is built on a cross-platform audio UI framework called JUCE. Because JUCE already knows how to draw on Linux screens, the entire graphical interface is the original Windows code running natively.

### What Was Swapped Out:
The only code we had to replace is the Windows-only "plumbing" that talks to Windows drivers, the Windows Registry, and Windows taskbars. All of these replacements live neatly isolated in the `linux/` directory, leaving the rest of the project completely untouched.

---

## 2. The 64-Bit Number Fix: Unlocking the Original Engine

The biggest hurdle in getting the original Windows sound engine to run on Linux came down to how computers count numbers.

Decades ago, when FxSound's core sound algorithms were first written, standard computers counted using 32-bit numbers (`long` data type). 
- When Windows transitioned to modern 64-bit computers, Microsoft decided that a `long` number would stay **32 bits wide** for backward compatibility (`LLP64` standard).
- When Linux transitioned to 64-bit computers, Linux decided that a `long` number should become **64 bits wide** (`LP64` standard).

Because the original DSP engine packed sound samples and algorithm settings assuming every `long` was 32 bits wide, running that same code on Linux originally resulted in scrambled numbers and broken audio.

Instead of rewriting hundreds of mathematical files, we solved this with one clean, surgical fix:
We told the compiler to treat numbers inside the sound engine as fixed 32-bit values on Linux (`pt_long` alias in `dsp/include/ptLong.h`). Instantly, the entire legacy audio engine snapped into place and ran perfectly on Linux with 100% mathematical accuracy.

---

## 3. Step-by-Step Comparison: Swapping Windows Parts for Linux Parts

| Feature | Windows FxSound | Linux Port (`fxsound-linux`) | Why the Linux Way is Better |
| :--- | :--- | :--- | :--- |
| **Audio Capture** | Custom kernel-level driver (`fxvad.sys`) | Built-in userspace virtual speaker via PipeWire (`linux/sndDevices_pw.cpp`) | No risky kernel modules or admin permissions needed. Runs safely in normal user space. |
| **Settings Storage** | Windows Registry (`HKEY_CURRENT_USER\Software\FxSound`) | Clean, in-memory cached INI file (`~/.config/FxSound/linux_registry.ini` in `linux/registry_posix.cpp`) | Easy to back up, edit in a text editor, or sync across machines without corrupting your system registry. |
| **System Tray** | Win32 notification area (`Shell_NotifyIcon`) | Modern Linux StatusNotifierItem over D-Bus (`linux/tray_sni.cpp`) | Works seamlessly across modern desktop environments (KDE Plasma, GNOME, Waybar, Sway) with native context menus. |
| **Background Running** | Hidden Windows Service or tray process | Systemd User Service (`packaging/fxsound.service`) | Standard Linux daemon management (`systemctl --user`) that starts with your desktop session. |
| **Volume Synchronization** | Windows OS audio session volume hooks | Bi-directional PipeWire volume mirroring (`linux/sndDevices_pw.cpp`) | Hardware volume knobs, OS volume sliders, and FxSound's virtual slider stay in sync without feedback loops. |
| **Global Shortcuts** | Win32 keyboard hooks (`RegisterHotKey`) | Wayland/X11 Desktop Portal (`org.freedesktop.portal.GlobalShortcuts`) & UNIX socket | Works even in full-screen games on Wayland where old X11 keyboard sniffing is blocked for security. |

---

## 4. Deep Dive on the Plumbing Replacements

### 1. Capturing Sound Without Kernel Drivers
On Windows, installing FxSound requires administrator privileges to install an audio driver (`fxvad.sys`). If that driver has a bug, it can crash your entire operating system with a blue screen.

On modern Linux, PipeWire already has built-in support for creating safe, virtual audio devices in userspace (`support.null-audio-sink`). FxSound on Linux simply tells PipeWire: *"Please create a virtual speaker for me, and route its output into my process."* There are zero kernel drivers, no root passwords required, and if anything ever goes wrong, it is completely impossible to crash your operating system.

### 2. Fast Settings Without Disk Thrashing
On Windows, settings like your active preset and slider positions are written to the Windows Registry. 

On Linux, FxSound mimics this registry interface using a simple text file (`linux_registry.ini`). To ensure your hard drive or SSD is never slowed down by constant reads and writes as you move sliders, the Linux port keeps all settings in super-fast computer memory (`std::mutex`-protected cache in `linux/registry_posix.cpp`) and flushes changes to disk cleanly and safely.

### 3. Tray Minimization & Background Audio
On Windows, closing the app window tucks FxSound into the system tray near the clock.

On Linux, the window closing button is hooked up to hide the graphical window while leaving the audio pipeline active (`hideMainWindow()` in `FxMainWindow.cpp`). The app registers a standard tray icon (`StatusNotifierItem` in `linux/tray_sni.cpp`). When you right-click the icon in your desktop panel (like KDE Plasma or GNOME via AppIndicator), your desktop draws a native menu allowing you to toggle power, switch presets, or cleanly quit.

### 4. Bi-Directional Volume Mirroring
When FxSound is your default audio output, what happens if you turn the physical volume wheel on your USB DAC or press the volume buttons on your keyboard?

- On Windows, the operating system manages this behind the scenes.
- On Linux, FxSound's PipeWire backend (`linux/sndDevices_pw.cpp`) watches both the virtual speaker and your physical sound card. If you change the volume on your physical DAC, FxSound catches the change and updates the virtual speaker. If you change volume in the app, FxSound updates the physical speaker. A smart safety guard prevents infinite feedback loops.
