#pragma once
#include <Arduino.h>

#include "Note.h"

const uint8_t MAX_NOTES_PER_STEP = 4;

// This struct represents a step in the sequencer, containing multiple notes and their properties.
//  Example usage: Step step1 = Step({Note(60, 90, 0), Note(62, 90, 0)}, 2, 500, 100); // Creates a step with two notes, each with a duration of 500ms and a delay of 100ms after the step.
struct Step {
  Note notes[MAX_NOTES_PER_STEP];
  uint8_t noteCount = 0;
  uint16_t duration = 250;
  uint16_t delayAfter = 100;

  Step() = default;  // <-- add this
  Step::Step(const Note* inputNotes, uint8_t count, uint16_t duration, uint16_t delayAfter)
    : noteCount(0), duration(duration), delayAfter(delayAfter)
{
    for (uint8_t i = 0; i < count && i < MAX_NOTES_PER_STEP; ++i) {
        notes[noteCount++] = inputNotes[i];
    }
}


  bool addNote(const Note& note) {
    if (noteCount >= MAX_NOTES_PER_STEP) return false;
    notes[noteCount++] = note;
    return true;
  }

  void clearNotes() {
    noteCount = 0;
  }
};
