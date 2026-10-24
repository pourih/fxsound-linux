# FxSound — Linux port (Fedora / PipeWire / Wayland)

This is a **Linux port** of FxSound (the Windows audio enhancer), maintained on
the `linux` branch. The Windows build is untouched (see `main` / the original
README below); all Linux work lives behind `JUCE_WINDOWS` guards or in new
`linux/` sources.

**Status:** the port runs end-to-end on Fedora 44 (KDE Wayland): the JUCE GUI
builds with GCC, the full DSP engine is validated, and audio flows system-wide
through a PipeWire virtual sink ("FxSound Audio Enhancer") into the real
output — no kernel driver needed (the Windows fxvad driver is replaced by a
userspace `support.null-audio-sink` + loopback). CLI (`--status`, `--power`,
`--preset`, `--set_effect`, `--output`, `--view`, `--scale`, ...), presets,
hotkey parity notes, desktop entry and RPM packaging are in place.

| Topic | Where |
|---|---|
| Quick build + run (Fedora) | [BUILD.md](BUILD.md) |
| Port design, decisions, blockers | [docs/linux-port/](docs/linux-port/) (start with RECON.md, DECISIONS.md) |
| PipeWire backend | [linux/sndDevices_pw.cpp](linux/sndDevices_pw.cpp), [linux/AudioPassthru_linux.cpp](linux/AudioPassthru_linux.cpp) |
| DSP engine (unchanged, LP64-fixed) | [dsp/](dsp/) — see DECISIONS.md "fixed-width DSP long pass" |
| Packaging | [packaging/fxsound.spec](packaging/fxsound.spec), [tools/install-desktop-entry.sh](tools/install-desktop-entry.sh) |

Linux highlights: XDG settings/presets/state, single-instance CLI over a UNIX
socket (JUCE 6's Linux forwarding is a stub), SIGTERM clean-quit, per-user UI
scale (`--scale=<f>` or `FXSOUND_SCALE`), freedesktop notifications, taskbar
icon via desktop entry + `_NET_WM_ICON`.

Licensing: AGPL-3.0-or-later, same as upstream (see [LICENSE](LICENSE)).
The Windows virtual-audio driver is **not** ported (MS-LPL; unnecessary —
PipeWire provides the virtual device in userspace).

---

# FxSound (original Windows app)


FxSound is a digital audio program built for Windows PC's. The background processing, built on a high-fidelity audio engine, acts as a sort of digital soundcard for your system. This means that your signals will have the clean passthrough when FxSound is active. There are active effects for shaping and boosting your sound's volume, timbre, and equalization included on top of this clean processing, allowing you to customize and enhance your sound.

## General Information
* Website: https://www.fxsound.com
* Installer: https://download.fxsound.com/fxsoundlatest
* Source code: https://github.com/fxsound2/fxsound-app
* Issue tracker: https://github.com/fxsound2/fxsound-app/issues
* Forum: https://forum.fxsound.com
* [Donate to FxSound](https://www.paypal.com/donate/?hosted_button_id=JVNQGYXCQ2GPG)

## Documentation

- [Command Line Options](docs/COMMAND_LINE_OPTIONS.md)
  
## Build Instructions
### Prerequisites
* Download and install the [latest version of FxSound](https://download.fxsound.com/fxsoundlatest)
* Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs)
* Install [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk)
* Install [JUCE 6.1.6](https://github.com/juce-framework/JUCE/releases/tag/6.1.6) for x64/x86
* Install [Latest JUCE framework](https://api.juce.com/api/v1/download/juce/latest/windows) for ARM64

FxSound application requires FxSound Audio Enhancer virtual audio driver. So, to run FxSound application built from source, we need to install FxSound which installs the audio driver.

### Build FxSound from Visual Studio
* Open [fxsound/Project/FxSound.sln](https://github.com/fxsound2/fxsound-app/blob/main/fxsound/Project/FxSound.sln) in Visual Studio
* Build the required configuration and platform and run

### Build after exporting the project form Projucer
FxSound application has three components. 
1. FxSound GUI application which uses JUCE framework
2. Audiopassthru module which is used by the application to interact with the audio devices
3. DfxDsp module which is the DSP for processing audio

Due to the some limitations with Projucer, after exporting the Visual Studio solution from Projucer, few changes have to be made in the solution to build FxSound.
1. Since the audiopassthru and DfxDsp dependency projects cannot be added to the solution when FxSound.sln is exported, open fxsound/Project/FxSound.sln in Visual Studio and add the existing projects audiopassthru/audiopassthru.vcxproj, dsp/DfxDsp.vcxproj.
2. From FxSound_App project, add reference to audiopassthru and DfxDsp.
3. If you run FxSound from Visual Studio, to let the application to use the presets, set the Working Directory to ```$(SolutionDir)..\..\bin\$(PlatformTarget)``` in FxSound_App Project->Properties->Debugging setting.

## How to contribute
We welcome anyone who wants to contribute to this project. For more details on how to contribute, follow [this contributing guideline](./CONTRIBUTING.md).

## Acknowledgements
Our special thanks to Advanced Installer for supporting us with Advanced Installer Professional license to build our installer.

[![image](https://github.com/user-attachments/assets/c133fe06-619c-4c17-bce9-f1cf051c5265)](https://www.advancedinstaller.com)

This project uses the [JUCE](https://juce.com) framework, which is licensed under [AGPL v3.0](https://github.com/juce-framework/JUCE/blob/master/LICENSE.md).

Thanks to [Theremino](https://www.theremino.com) for the valuable contributions they do through major feature enhancements in FxSound.

## License
[AGPL v3.0](https://github.com/fxsound2/fxsound-app/blob/main/LICENSE)
