#pragma once

#include "config.h"

struct LFO {
  float phase, freq;
  uint8_t minVal, maxVal;
};

LFO velLFO = {0.0, 0.1, BASE_VEL - 20, BASE_VEL + 20};
LFO volLFO = {0.5, 0.05, 40, 100};

uint8_t updateLFO(LFO& lfo, unsigned long dt) {
  lfo.phase += lfo.freq * (dt / 1000.0);
  if (lfo.phase >= 1.0) lfo.phase -= 1.0;
  float s = sin(2 * PI * lfo.phase);
  float mid = (lfo.maxVal + lfo.minVal) / 2.0;
  float amp = (lfo.maxVal - lfo.minVal) / 2.0;
  return constrain((int)(mid + amp * s), 0, 127);
}
