# JUCE patches (Phase 2 rule b)

Target: JUCE 6.1.6 (the version `.jucer` pins, so no shared-code drift).
Location: `third_party/JUCE` submodule (exact commit TBD on Fedora — see DECISIONS.md).

Rules:
- Only small GCC 16 compat fixes here, one file per upstream issue.
- If GUI target needs >~10 patches, switch Linux to JUCE 7.0.12 per plan rule c.
- Never FetchContent — submodule exact commit only.

Current status: UNVERIFIED on Linux. No patches yet.
