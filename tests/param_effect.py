#!/usr/bin/env python3
"""param_effect.py — assert each DfxDsp effect audibly changes output (Phase 6).
Runs dsp_cli on one input with --power=1 vs each effect at max, diffs WAVs.
Fails if any effect output == bypass (catches UI with no audible effect).
Usage: python3 tests/param_effect.py --dsp-cli build/dsp_cli --in tests/golden/inputs/pink_10s.wav --tmp /tmp/fx
Untested — run on Windows + Fedora.
"""
import argparse, subprocess, wave, struct, os, sys
EFFECTS = ["fidelity=10", "ambience=10", "surround=10", "dynamicboost=10", "bass=10"]

def read_pcm16(path):
    w = wave.open(path, 'rb')
    raw = w.readframes(w.getnframes())
    return struct.unpack('<' + 'h' * (len(raw) // 2), raw)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--dsp-cli', required=True)
    ap.add_argument('--in', dest='inp', required=True)
    ap.add_argument('--tmp', required=True)
    ap.add_argument('--preset', default='')
    a = ap.parse_args()
    os.makedirs(a.tmp, exist_ok=True)
    base = os.path.join(a.tmp, 'bypass.wav')
    cmd = [a.dsp_cli, f'--in={a.inp}', f'--out={base}', '--power=1']
    if a.preset: cmd.append(f'--preset={a.preset}')
    subprocess.check_call(cmd)
    ref = read_pcm16(base)
    fails = 0
    for e in EFFECTS:
        out = os.path.join(a.tmp, f"fx_{e.split('=')[0]}.wav")
        cmd = [a.dsp_cli, f'--in={a.inp}', f'--out={out}', '--power=1', f'--effect={e}']
        if a.preset: cmd.append(f'--preset={a.preset}')
        subprocess.check_call(cmd)
        got = read_pcm16(out)
        diff = sum(1 for x, y in zip(ref, got) if x != y)
        frac = diff / len(ref)
        ok = frac > 0.001
        print(f"{'OK' if ok else 'FAIL'} {e}: {diff}/{len(ref)} differ ({100*frac:.2f}%)")
        if not ok: fails += 1
    print("RESULT:", "PASS" if fails == 0 else f"FAIL {fails}")
    sys.exit(1 if fails else 0)

if __name__ == '__main__':
    main()
