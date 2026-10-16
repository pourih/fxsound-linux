#!/usr/bin/env python3
"""gen_inputs.py — generate fixed test WAV set for golden masters.
Outputs 48kHz stereo 16-bit: sine sweep 20-20k 10s, pink noise 10s, full-scale clipping 5s.
Music excerpt NOT included (copyright) — user supplies music_excerpt.wav (any 10s stereo).
Run: python3 tools/golden/gen_inputs.py --dir tests/golden/inputs/
Untested — verify sample counts on Windows.
"""
import argparse, wave, struct, math, random
def write_wav(path, samples, sr=48000, ch=2):
    w = wave.open(path, 'wb'); w.setnchannels(ch); w.setsampwidth(2); w.setframerate(sr)
    w.writeframes(struct.pack('<' + 'h' * len(samples), *samples)); w.close()
def sweep(sr, dur):
    n = sr * dur; out = []
    for i in range(n):
        f = 20 * (20000 / 20) ** (i / n)
        v = int(0.5 * 32767 * math.sin(2 * math.pi * f * i / sr))
        out += [v, v]
    return out
def pink(sr, dur):
    random.seed(0); n = sr * dur; out = []; b = [0.0] * 7
    for _ in range(n):
        w = random.uniform(-1, 1)
        b[0] = 0.99886 * b[0] + w * 0.0555179
        b[1] = 0.99332 * b[1] + w * 0.0750759
        b[2] = 0.96900 * b[2] + w * 0.1538520
        b[3] = 0.86650 * b[3] + w * 0.3104856
        b[4] = 0.55000 * b[4] + w * 0.5329522
        b[5] = -0.7616 * b[5] - w * 0.0168980
        v = sum(b) * 0.11
        s = max(-1.0, min(1.0, v))
        out += [int(s * 32767 * 0.5)] * 2
    return out
def clip(sr, dur):
    n = sr * dur; out = []
    for i in range(n):
        v = 32767 if (i // (sr // 2)) % 2 == 0 else -32768
        # 1kHz square at full scale to catch clamping
        out += [v, v]
    return out[:n * 2]
def main():
    ap = argparse.ArgumentParser(); ap.add_argument('--dir', required=True); a = ap.parse_args()
    import os; os.makedirs(a.dir, exist_ok=True)
    write_wav(os.path.join(a.dir, 'sweep_20_20k_10s.wav'), sweep(48000, 10))
    write_wav(os.path.join(a.dir, 'pink_10s.wav'), pink(48000, 10))
    write_wav(os.path.join(a.dir, 'clip_square_5s.wav'), clip(48000, 5))
    print("wrote sweep/pink/clip. Add music_excerpt.wav (10s stereo 48k) manually.")
if __name__ == '__main__': main()
