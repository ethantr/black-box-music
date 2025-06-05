#include "LFO.h"

LFO::LFO(float startingFreq, uint8_t minV, uint8_t maxV)
  : phase(0.0), freq(startingFreq), minVal(minV), maxVal(maxV), lastTime(0) 
{}

void LFO::begin() {
  lastTime = millis();
}

// update(): compute elapsed time, advance phase, compute sinusoidal output
uint8_t LFO::update() {
  unsigned long now = millis();
  unsigned long dt = now - lastTime;
  lastTime = now;

  // advance phase
  float deltaPhase = freq * (dt / 1000.0);
  phase += deltaPhase;
  if (phase >= 1.0) phase -= 1.0;

  // sine from 0..2π
  float angle = phase * 2.0 * PI;
  float s = sin(angle);  // -1..1

  // map to minVal..maxVal
  float mid = (maxVal + minVal) / 2.0;
  float amp = (maxVal - minVal) / 2.0;
  float value = mid + amp * s;
  if (value < 0) value = 0;
  if (value > 127) value = 127;
  return (uint8_t)(value + 0.5);
}

void LFO::setFreq(float f) {
  freq = f;
}

void LFO::setDepth(uint8_t minV, uint8_t maxV) {
  minVal = minV;
  maxVal = maxV;
}
