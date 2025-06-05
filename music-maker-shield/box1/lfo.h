#pragma once
#include <Arduino.h>

/*
  LFO (low‐frequency oscillator) for generating a continuous CC value (0–127).
  • phase, freq: control the speed (Hz) of the sine wave.
  • minVal, maxVal: define the depth (min..max) in MIDI range (0..127).
  • Call begin() in setup() to initialize the timing.
  • On each loop(), call update() to get the current value (mapped 0..127).
  • You can adjust freq or depth any time via setFreq() or setDepth().
*/

class LFO {
public:
  LFO(float startingFreq = 0.2, uint8_t minV = 20, uint8_t maxV = 80);

  // Must call in setup() once
  void begin();

  // Call once per loop; returns a uint8_t in [minVal..maxVal]
  uint8_t update();

  // Change speed (Hz)
  void setFreq(float f);

  // Change depth (min..max)
  void setDepth(uint8_t minV, uint8_t maxV);

private:
  float phase;            // 0..1 cycle
  float freq;             // cycles per second
  uint8_t minVal, maxVal; // range for output
  unsigned long lastTime; // last millis() when update was called
};
