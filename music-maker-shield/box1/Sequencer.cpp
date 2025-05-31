#include "Sequencer.h"
#include <Arduino.h>

Sequencer::Sequencer(uint16_t bpm, MidiInterface* midi)
  : bpm(bpm), midi(midi) {}

bool Sequencer::addStep(const Step& step) {
  if (stepCount >= MAX_STEPS) return false;
  steps[stepCount++] = step;
  return true;
}

void Sequencer::setStep(uint8_t index, const Step& step) {
  if (index < MAX_STEPS) steps[index] = step;
}

void Sequencer::clear() {
  stepCount = 0;
}

void Sequencer::reset() {
  currentStep = 0;
  stepActive = false;
  stepTimer = millis();
}

void Sequencer::setTempo(uint16_t newBPM) {
  bpm = newBPM;
}

void Sequencer::update() {
  if (stepCount == 0) return;

  unsigned long now = millis();
  if (!stepActive) {
    playStep(steps[currentStep]);
    stepTimer = now + steps[currentStep].duration;
    stepActive = true;
  } else if (now >= stepTimer) {
    for (uint8_t i = 0; i < steps[currentStep].noteCount; ++i) {
      const Note& note = steps[currentStep].notes[i];
      midi->noteOff(note.channel, note.pitch, 0);
    }
    stepTimer = now + steps[currentStep].delayAfter;
    stepActive = false;
    currentStep = (currentStep + 1) % stepCount;
  }
}

void Sequencer::playStep(const Step& s) {
  for (uint8_t i = 0; i < s.noteCount; ++i) {
    const Note& note = s.notes[i];
    midi->noteOn(note.channel, note.pitch, note.velocity);
  }
}
