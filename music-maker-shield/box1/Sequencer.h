#pragma once
#include <Arduino.h>
#include "Step.h"
#include "MidiInterface.h"

const uint8_t MAX_STEPS = 16;

class Sequencer {
public:
  Sequencer(uint16_t bpm, MidiInterface* midi);

  bool addStep(const Step& step);
  void setStep(uint8_t index, const Step& step);
  void clear();
  void reset();
  void update();
  void setTempo(uint16_t bpm);

private:
  void playStep(const Step& step);

  uint16_t bpm;
  Step steps[MAX_STEPS];
  uint8_t stepCount = 0;
  uint8_t currentStep = 0;
  bool stepActive = false;
  unsigned long stepTimer = 0;
  MidiInterface* midi;
};
