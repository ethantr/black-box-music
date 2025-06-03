#include "Sequencer.h"
#include <Arduino.h>

const int LED_PIN = 6;

int ledFadeDuration = 0;
int ledTargetBrightness = 0;
int ledStartBrightness = 0;
unsigned long ledFadeStart = 0;

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

void Sequencer::setDuration(uint16_t d, uint8_t index) { 
  steps[index].duration = d;
}

void Sequencer::setVelocity(uint8_t v, uint8_t index) { 
  for (uint8_t i = 0; i < steps[index].noteCount; ++i) {
    steps[index].notes[i].velocity = v;
  }
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
    ledStartBrightness = ledTargetBrightness;
  ledTargetBrightness = 0;
  ledFadeStart = now;
  ledFadeDuration = 200;
  }

  // Smooth LED fade
  unsigned long fadeElapsed = millis() - ledFadeStart;
  if (fadeElapsed < ledFadeDuration) {
    float t = (float)fadeElapsed / ledFadeDuration;
    uint8_t brightness = ledStartBrightness + (ledTargetBrightness - ledStartBrightness) * t;
    analogWrite(LED_PIN, brightness);
  } else {
    analogWrite(LED_PIN, ledTargetBrightness);  // ensure final value is set
  }
}

void Sequencer::playStep(const Step& s) {

  ledStartBrightness = analogRead(LED_PIN) / 4; // current LED brightness (approximate)
  ledTargetBrightness = map(s.duration, 300, 3000, 50, 255);
  ledTargetBrightness = constrain(ledTargetBrightness, 0, 255);
  ledFadeStart = millis();
  ledFadeDuration = 200;  // adjust fade speed (ms)


  for (uint8_t i = 0; i < s.noteCount; ++i) {
    const Note& note = s.notes[i];
    midi->noteOn(note.channel, note.pitch, note.velocity);
  }
}
