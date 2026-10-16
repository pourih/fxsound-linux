#!/usr/bin/env bash
# portal_shortcut_spike.sh — Hotkeys v1.1 [USER spike] (Phase 9).
# Tries org.freedesktop.portal.GlobalShortcuts with the five FxSound command IDs
# as a small standalone test (python sdbus / gdbus), reports whether KDE shows
# and binds them. Skip XGrabKey unless X11 sessions are targeted.
# Run on Fedora 44 KDE; paste output + screenshots into DESKTOP.md results.
# Untested here.
set -u
echo "session: ${XDG_SESSION_TYPE:-unknown}"
echo "IDs: cmd_on_off, cmd_open_close, cmd_next_preset, cmd_previous_preset, cmd_change_output"
echo "---"
echo "MANUAL step 1: list portal:"
echo "  gdbus introspect --session --dest org.freedesktop.portal.Desktop --object-path /org/freedesktop/portal/desktop | grep -i -A3 GlobalShortcuts"
echo "MANUAL step 2 (python sdbus if installed, else gdbus call BindShortcuts):"
echo "  Request BindShortcuts for the 5 IDs, check KDE Global Shortcuts UI shows FxSound entries."
echo "MANUAL step 3: bind one (e.g. Ctrl+Alt+F) in KDE settings, press it, confirm portal"
echo "  Activated signal arrives. Record YES/NO per ID below."
echo "--- results template ---"
echo "KDE shows FxSound shortcuts: YES/NO"
echo "Bind works: YES/NO"
echo "Activated received: YES/NO"
echo "Decision: keep CLI-shortcut hotkeys v1 (canSupportHotkeys=false) unless all YES."
