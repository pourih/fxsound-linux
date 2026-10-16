# PACKAGING + LICENSING (Phase 10)

## RPM
- Spec: `packaging/fxsound.spec` (`License: AGPL-3.0-or-later`).
  Installs: `/usr/bin/fxsound`, `/usr/share/fxsound/Factsoft/*.fac`,
  `.desktop`, icons, AppStream metainfo. No services/drivers/udev.
- COPR: push this branch to a Fedora 44 chroot; `rpmbuild -ba packaging/fxsound.spec`
  in CI planned (UNVERIFIED here). Flatpak deferred (system sink fits poorly in sandbox).
- [USER] Done when: `rpmbuild` succeeds in CI, RPM installs on clean Fedora 44
  KDE VM, app runs from `/usr/bin/fxsound` with factory presets from `/usr/share`.

## First-run seeding
- `linux/paths_posix.cpp`: factory `/usr/share/fxsound/Factsoft` (+ exe-adjacent
  fallback for uninstalled runs); user `~/.local/share/fxsound/presets/`;
  reference defaults from `/usr/share/fxsound/FxSound.settings` copied to
  `~/.config/FxSound/` on first run if absent.

## License (from RECON §1)
- App `LICENSE` + all sampled headers = AGPL-3.0-or-later (not GPLv3).
  Legacy `dsp/ptutil/Qnt/u_qnt.h:1-5` Power Technology 1994-1997 header is the
  only pre-FxSound notice found; treat as compatible data-file comment unless
  legal review says otherwise — full-tree legacy audit still UNSURE.
- JUCE 6.1.6 per README is claimed AGPL v3.0 in repo; distributed binary falls
  under AGPLv3 terms including source offer. About box + LICENSE files must
  state AGPL-3.0-or-later and point to source. No JUCE commercial license held.
- Driver MS-LPL (`fxsound-driver/license.rtf`) does NOT apply — driver not ported.
