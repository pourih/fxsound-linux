#!/usr/bin/env python3
"""compare_golden.py — compare dsp_cli output to Windows reference.
Default max abs error 1 LSB for int16; also report bit-exact fraction.
Usage: python3 tests/compare_golden.py --manifest tests/golden/manifest.json --out-dir out/
Untested on Linux — verify on Windows self-run first (should pass against itself).
"""
import argparse, json, wave, sys, os

def read_pcm16(path):
    w = wave.open(path, 'rb')
    assert w.getsampwidth() == 2, f"{path}: only 16-bit"
    n = w.getnframes()
    raw = w.readframes(n)
    import struct
    fmt = '<' + 'h' * (len(raw) // 2)
    vals = struct.unpack(fmt, raw)
    return vals, w.getframerate(), w.getnchannels()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--manifest', required=True)
    ap.add_argument('--out-dir', required=True)
    ap.add_argument('--tol-lsb', type=int, default=1)
    args = ap.parse_args()
    m = json.load(open(args.manifest))
    fails = 0
    total_exact = 0
    total_samp = 0
    for case in m['cases']:
        ref = os.path.join(os.path.dirname(args.manifest), case['ref'])
        got = os.path.join(args.out_dir, case['out'])
        if not os.path.exists(got):
            print(f"MISSING {got}"); fails += 1; continue
        r, rsr, rch = read_pcm16(ref)
        g, gsr, gch = read_pcm16(got)
        if rsr != gsr or rch != gch or len(r) != len(g):
            print(f"MISMATCH meta {case['out']}: ref {rsr}/{rch}/{len(r)} vs got {gsr}/{gch}/{len(g)}")
            fails += 1; continue
        maxabs = 0
        exact = 0
        for a, b in zip(r, g):
            d = abs(a - b)
            if d > maxabs: maxabs = d
            if d == 0: exact += 1
        total_exact += exact; total_samp += len(r)
        status = "OK" if maxabs <= args.tol_lsb else "FAIL"
        if status == "FAIL": fails += 1
        print(f"{status} {case['out']}: maxabs={maxabs} LSB exact={exact}/{len(r)}={100.0*exact/len(r):.2f}%")
    print(f"bit-exact total: {total_exact}/{total_samp}={100.0*total_exact/total_samp if total_samp else 0:.2f}%")
    print("RESULT:", "PASS" if fails == 0 else f"FAIL {fails}")
    sys.exit(1 if fails else 0)

if __name__ == '__main__':
    main()
