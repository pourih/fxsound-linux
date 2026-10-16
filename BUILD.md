# BUILD — Fedora 44 KDE, beginner, one command per line

# 1. Update system.
sudo dnf update -y
# 2. Install build tools and libraries.
sudo dnf install -y gcc-c++ cmake ninja-build git pkgconf-pkg-config pipewire-devel systemd-devel dbus-devel alsa-lib-devel libX11-devel libXext-devel libXinerama-devel libXrandr-devel libXcursor-devel libXcomposite-devel libXrender-devel freetype-devel fontconfig-devel mesa-libGL-devel libcurl-devel libasan libubsan gdb valgrind pipewire-utils wireplumber pulseaudio-utils
# 3. Clone repo and branch.
git clone https://github.com/fxsound2/fxsound-app.git fxsound-app
# 4. Enter repo.
cd fxsound-app
# 5. Switch to Linux port branch.
git checkout linux
# 6. Init submodules (Resources strings; JUCE below).
git submodule update --init --recursive
# 7. Add JUCE 6.1.6 submodule at exact commit (ask maintainer for commit hash; record in docs/linux-port/DECISIONS.md).
git submodule add https://github.com/juce-framework/JUCE.git third_party/JUCE
# 8. Pin JUCE 6.1.6 tag commit (replace <commit> with 6.1.6 tag hash).
git -C third_party/JUCE checkout <commit>
# 9. Configure release build.
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
# 10. Build everything.
cmake --build build
# 11. Run tests.
ctest --test-dir build --output-on-failure
# 12. Run offline DSP smoke (needs Phase 4 golden inputs).
./build/dsp_cli --in tests/golden/inputs/pink_10s.wav --out /tmp/fx_out.wav --preset bin/x64/Factsoft/1.fac
# 13. Compare goldens (needs Windows refs from Phase 4).
python3 tests/compare_golden.py --manifest tests/golden/manifest.json --out-dir /tmp/fx_outdir
# 14. Parameter-effect check (each effect must audibly change output).
python3 tests/param_effect.py --dsp-cli ./build/dsp_cli --in tests/golden/inputs/pink_10s.wav --tmp /tmp/fx_param
# 15. Launch GUI (expect 12 presets, no audio until Phase 7 wired).
./build/FxSound
# 16. PipeWire spike (one-day scripted, saves spike_out/).
./tools/spike/pw_spike.sh
# 17. Soak test (8 hours, snapshots pw-top every 5 minutes).
./tools/soak.sh tests/golden/inputs/pink_10s.wav 8
# 18. Build RPM (clean VM install test).
rpmbuild -ba packaging/fxsound.spec

# Notes.
# All Linux code is UNVERIFIED here (no Linux box) — fix errors, log decisions in docs/linux-port/DECISIONS.md, failures twice in docs/linux-port/BLOCKERS.md then stop.
# Windows .jucer/.vcxproj/.sln untouched. compat/ only for DfxDsp/ptutil/AudioPassthruCore, never PipeWire code.
# Out of scope v1: Installer/, fxmcp, fxdiag, ARM, Flatpak, JACK.
