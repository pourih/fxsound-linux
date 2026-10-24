#!/bin/sh
# FxSound Linux port — register the app menu entry + taskbar icon for a
# development checkout (the RPM install does this system-wide already).
# KWin matches the window's WM_CLASS (FxSound) to StartupWMClass below and
# uses the hicolor icon for the taskbar, independent of _NET_WM_ICON.
set -e
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
mkdir -p ~/.local/share/applications
mkdir -p ~/.local/share/icons/hicolor/48x48/apps
mkdir -p ~/.local/share/icons/hicolor/256x256/apps
sed "s|^Icon=.*|Icon=fxsound|" "$ROOT/packaging/fxsound.desktop" > ~/.local/share/applications/fxsound.desktop
install -m0644 "$ROOT/fxsound/Images/fxsound.png" ~/.local/share/icons/hicolor/48x48/apps/fxsound.png
install -m0644 "$ROOT/fxsound/Images/fxsound_large.png" ~/.local/share/icons/hicolor/256x256/apps/fxsound.png
update-desktop-database ~/.local/share/applications 2>/dev/null || true
gtk-update-icon-cache ~/.local/share/icons/hicolor 2>/dev/null || true
echo "installed: ~/.local/share/applications/fxsound.desktop + hicolor icons"
