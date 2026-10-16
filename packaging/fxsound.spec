Name:           fxsound
Version:        1.2.15
Release:        1%{?dist}
Summary:        FxSound audio enhancer (Linux port, PipeWire)
License:        AGPL-3.0-or-later
URL:            https://github.com/fxsound2/fxsound-app
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  gcc-c++ cmake ninja-build pkgconf-pkg-config
BuildRequires:  pipewire-devel systemd-devel dbus-devel alsa-lib-devel
BuildRequires:  libX11-devel libXext-devel libXinerama-devel libXrandr-devel
BuildRequires:  libXcursor-devel libXcomposite-devel libXrender-devel
BuildRequires:  freetype-devel fontconfig-devel mesa-libGL-devel libcurl-devel

Requires:       pipewire wireplumber pipewire-utils
# No services, drivers, or udev rules (null sink lives only while app runs).

%description
System-wide audio enhancer. Apps play to the "FxSound Audio Enhancer" null
sink (PipeWire), DSP processes, output goes to the real device. JUCE GUI.
Combined work is AGPL-3.0-or-later (app AGPL + JUCE AGPL; see LICENSE).
Driver MS-LPL does not apply — driver is not ported. Flatpak deferred.

%prep
%autosetup -n %{name}-%{version}

%build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

%install
install -Dm0755 build/FxSound %{buildroot}%{_bindir}/fxsound
install -Dm0644 packaging/fxsound.desktop %{buildroot}%{_datadir}/applications/fxsound.desktop
install -Dm0644 packaging/com.fxsound.FxSound.metainfo.xml %{buildroot}%{_datadir}/metainfo/com.fxsound.FxSound.metainfo.xml
install -Dm0644 fxsound/Images/fxsound.png %{buildroot}%{_datadir}/icons/hicolor/48x48/apps/fxsound.png
install -Dm0644 fxsound/Images/fxsound_large.png %{buildroot}%{_datadir}/icons/hicolor/256x256/apps/fxsound.png
mkdir -p %{buildroot}%{_datadir}/fxsound/Factsoft
install -m0644 bin/x64/Factsoft/*.fac %{buildroot}%{_datadir}/fxsound/Factsoft/
# Default settings seeded per-user on first run (see linux/paths_posix.cpp); also ship reference:
install -Dm0644 Installer/Resources/FxSound.settings %{buildroot}%{_datadir}/fxsound/FxSound.settings

%files
%{_bindir}/fxsound
%{_datadir}/applications/fxsound.desktop
%{_datadir}/metainfo/com.fxsound.FxSound.metainfo.xml
%{_datadir}/icons/hicolor/*/apps/fxsound.png
%{_datadir}/fxsound/Factsoft/
%{_datadir}/fxsound/FxSound.settings
%license LICENSE

%changelog
* Thu Oct 16 2026 Linux Port <port@localhost> - 1.2.15-1
- Initial Fedora 44 PipeWire port (UNVERIFIED here; see docs/linux-port/).
