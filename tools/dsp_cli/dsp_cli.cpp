// dsp_cli — portable offline DSP runner for golden masters (Phase 4) + Linux validation (Phase 6).
// Windows + Linux, C++17, no Win32, no JUCE. WAV 16-bit PCM only.
// Usage:
//   dsp_cli --in in.wav --out out.wav --preset factory/1.fac [--power 1]
//           [--effect bass=7.5,ambience=4.0] [--bands 10] [--gain 0:3.0,1:-2.5]
// Effects: fidelity|clarity,ambience,surround,dynamicboost|dynamic_boost,bass|bassboost|bass_boost (0..10).
// Untested on Linux — verify against Windows references via tests/compare_golden.py.
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include "DfxDsp.h"

namespace fs = std::filesystem;

#pragma pack(push, 1)
struct WavHdr {
  char riff[4]; uint32_t size; char wave[4]; char fmt[4]; uint32_t fmtLen;
  uint16_t audioFmt; uint16_t ch; uint32_t sr; uint32_t byteRate;
  uint16_t blockAlign; uint16_t bps; char data[4]; uint32_t dataLen;
};
#pragma pack(pop)

static bool readWav(const std::string& p, std::vector<int16_t>& samples, uint32_t& sr, uint16_t& ch) {
  FILE* f = nullptr;
#ifdef _WIN32
  fopen_s(&f, p.c_str(), "rb");
#else
  f = fopen(p.c_str(), "rb");
#endif
  if (!f) { fprintf(stderr, "open in failed: %s\n", p.c_str()); return false; }
  WavHdr h{};
  if (fread(&h, sizeof(h), 1, f) != 1) { fclose(f); return false; }
  if (memcmp(h.riff, "RIFF", 4) || memcmp(h.wave, "WAVE", 4) || h.audioFmt != 1 || h.bps != 16) {
    fprintf(stderr, "only 16-bit PCM WAV supported\n"); fclose(f); return false;
  }
  // skip extra fmt bytes
  if (h.fmtLen > 16) fseek(f, (long)(h.fmtLen - 16), SEEK_CUR);
  // find data chunk (assume next)
  if (memcmp(h.data, "data", 4) != 0) { fprintf(stderr, "no data chunk\n"); fclose(f); return false; }
  size_t n = h.dataLen / 2;
  samples.resize(n);
  if (fread(samples.data(), 2, n, f) != n) { fclose(f); return false; }
  fclose(f); sr = h.sr; ch = h.ch; return true;
}

static bool writeWav(const std::string& p, const std::vector<int16_t>& samples, uint32_t sr, uint16_t ch) {
  FILE* f = nullptr;
#ifdef _WIN32
  fopen_s(&f, p.c_str(), "wb");
#else
  f = fopen(p.c_str(), "wb");
#endif
  if (!f) { fprintf(stderr, "open out failed\n"); return false; }
  WavHdr h{};
  memcpy(h.riff, "RIFF", 4); memcpy(h.wave, "WAVE", 4); memcpy(h.fmt, "fmt ", 4); memcpy(h.data, "data", 4);
  h.fmtLen = 16; h.audioFmt = 1; h.ch = ch; h.sr = sr; h.bps = 16;
  h.blockAlign = (uint16_t)(ch * 2); h.byteRate = sr * h.blockAlign;
  h.dataLen = (uint32_t)(samples.size() * 2); h.size = 36 + h.dataLen;
  fwrite(&h, sizeof(h), 1, f);
  fwrite(samples.data(), 2, samples.size(), f);
  fclose(f); return true;
}

static std::wstring toWide(const std::string& s) {
  // filesystem-native conversion;足夠 for ASCII preset paths used in goldens.
  return fs::u8path(s).wstring();
}

static int effectId(const std::string& n) {
  if (n=="fidelity"||n=="clarity") return 0;
  if (n=="ambience") return 1;
  if (n=="surround") return 2;
  if (n=="dynamicboost"||n=="dynamic_boost") return 3;
  if (n=="bass"||n=="bassboost"||n=="bass_boost") return 4;
  return -1;
}

int main(int argc, char** argv) {
  std::string in, out, preset, effects, gains;
  int power = 1, bands = -1;
  for (int i = 1; i < argc; ++i) {
    std::string a = argv[i];
    auto val = [&](const char* k) { size_t n = strlen(k); return a.size() > n && a.compare(0, n, k) == 0 ? a.substr(n) : std::string(); };
    std::string v;
    if ((v = val("--in=")).size()) in = v;
    else if ((v = val("--out=")).size()) out = v;
    else if ((v = val("--preset=")).size()) preset = v;
    else if ((v = val("--effect=")).size() || (v = val("--set_effect=")).size()) effects = v;
    else if ((v = val("--power=")).size()) power = atoi(v.c_str());
    else if ((v = val("--bands=")).size() || (v = val("--num_bands=")).size()) bands = atoi(v.c_str());
    else if ((v = val("--gain=")).size() || (v = val("--set_band_gain=")).size()) gains = v;
    else { fprintf(stderr, "unknown arg %s\n", a.c_str()); return 2; }
  }
  if (in.empty() || out.empty()) { fprintf(stderr, "need --in= --out= [--preset=] [--power=] [--effect=] [--bands=] [--gain=]\n"); return 2; }
  std::vector<int16_t> samples; uint32_t sr = 0; uint16_t ch = 0;
  if (!readWav(in, samples, sr, ch)) return 1;
  DfxDsp dsp;
  dsp.setSignalFormat(16, ch, (int)sr, 16);
  if (!preset.empty()) {
    if (dsp.loadPreset(toWide(preset)) != 0) fprintf(stderr, "warn: loadPreset failed\n");
  }
  if (bands > 0) dsp.setNumBands(bands);
  if (!effects.empty()) {
    // parse name:value[,name:value]
    size_t p = 0;
    while (p < effects.size()) {
      size_t c = effects.find(':', p); if (c == std::string::npos) break;
      size_t e = effects.find(',', c); if (e == std::string::npos) e = effects.size();
      std::string n = effects.substr(p, c - p); float v = (float)atof(effects.substr(c + 1, e - c - 1).c_str());
      int id = effectId(n);
      if (id >= 0) dsp.setEffectValue((DfxDsp::Effect)id, v);
      p = e + 1;
    }
  }
  if (!gains.empty()) {
    size_t p = 0;
    while (p < gains.size()) {
      size_t c = gains.find(':', p); if (c == std::string::npos) break;
      size_t e = gains.find(',', c); if (e == std::string::npos) e = gains.size();
      int b = atoi(gains.substr(p, c - p).c_str()); float v = (float)atof(gains.substr(c + 1, e - c - 1).c_str());
      dsp.setEqBandBoostCut(b, v);
      p = e + 1;
    }
  }
  dsp.powerOn(power != 0);
  size_t frames = samples.size() / ch;
  const size_t CHUNK = 16384;
  std::vector<int16_t> outBuf(samples.size());
  for (size_t f = 0; f < frames; f += CHUNK) {
    size_t n = frames - f < CHUNK ? frames - f : CHUNK;
    int sets = (int)n;
    dsp.processAudio(&samples[f * ch], &outBuf[f * ch], sets, 0);
  }
  if (!writeWav(out, outBuf, sr, ch)) return 1;
  return 0;
}
