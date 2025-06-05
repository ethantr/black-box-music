#pragma once
#include <Arduino.h>

// ChordGenerator:  
//  • Holds a small vocabulary of 3 four‐note chords (Ab, Bb, C in A-minor coloring).  
//  • Maintains a simple Markov transition matrix (3×3).  
//  • pickNext(...) uses the current chord index and a "pressure" reading (0..1023) 
//    to bias transitions (more likely to repeat under high pressure).  
//  • You can call regenerate() to fill an 8‐step progression.  

class ChordGenerator {
public:
  static const uint8_t CHORD_COUNT = 3;
  static const uint8_t NOTES_PER_CHORD = 4;
  static const uint8_t PROG_LENGTH = 8;

  // chordVoices[i][j] = MIDI pitch of chord i, note j
  //    chord 0 = Ab (C, Eb, G, low Ab), 
  //    chord 1 = Bb (D, F, G, low Bb),
  //    chord 2 = C (Eb, G, Ab, low C)
  static const uint8_t chordVoices[CHORD_COUNT][NOTES_PER_CHORD];

  // Base transition probabilities (row sums = 1.0).  
  // e.g. TRANS[0][1] = probability of going from chord 0→1
  static const float BASE_TRANS[CHORD_COUNT][CHORD_COUNT];

  // After biasing, this holds the modified transition for current chord.
  float workingTrans[CHORD_COUNT];

  // The generated progression of chord indices (0..2)
  uint8_t progression[PROG_LENGTH];

  // Track the current step in the progression
  uint8_t currentStep = 0;

  // Call in setup() once to seed random
  void begin() {
    randomSeed(analogRead(A5));
    regenerate(0);  // start at chord 0 by default
  }

  // Call this whenever you want to force‐regenerate the 8‐step progression.
  // biasRepeat = true means “force higher probability to stay on same chord.”
  void regenerate(bool biasRepeat = false) {
    // Start the first chord at random
    progression[0] = random(CHORD_COUNT);

    for (uint8_t i = 1; i < PROG_LENGTH; i++) {
      uint8_t prev = progression[i - 1];
      biasRow(prev, biasRepeat);
      progression[i] = pickIndexFrom(workingTrans, CHORD_COUNT);
    }
    currentStep = 0;
  }

  // Return the next chord index, stepping the internal cursor (wraps at PROG_LENGTH)
  uint8_t nextChord(int pressureValue) {
    // Before returning, we might re‐bias the transition row based on pressure
    uint8_t idx = progression[currentStep];
    float pNorm = constrain((float)pressureValue / 1023.0, 0.0, 1.0);
    biasRow(idx, false, pNorm);

    // Compute next index in progression
    uint8_t result = progression[currentStep];
    currentStep = (currentStep + 1) % PROG_LENGTH;
    return result;
  }

private:
  // Re‐compute workingTrans[] for row = prevIdx, blending between BASE_TRANS
  // and a “strong self‐transition” if biasRepeat==true. pBias (0..1) also pushes weight to self.
  void biasRow(uint8_t prevIdx, bool biasRepeat, float pBias = 0.0) {
    // Copy base row:
    for (uint8_t j = 0; j < CHORD_COUNT; j++) {
      workingTrans[j] = BASE_TRANS[prevIdx][j] * (1.0 - pBias);
    }
    // If biasing repeat, or pBias>0, add some to staying on the same chord:
    float repeatBoost = (biasRepeat ? 0.5 : 0.0) * (1.0 - pBias) + pBias * 0.5;
    workingTrans[prevIdx] += repeatBoost;

    // Normalize to sum=1
    float sum = 0;
    for (uint8_t j = 0; j < CHORD_COUNT; j++) sum += workingTrans[j];
    if (sum <= 0) sum = 1.0;
    for (uint8_t j = 0; j < CHORD_COUNT; j++) {
      workingTrans[j] /= sum;
    }
  }

  // Helper: pick an index 0..(n-1) according to probabilities in arr[] (summing to 1)
  uint8_t pickIndexFrom(const float arr[], uint8_t n) {
    float r = random(10000) / 10000.0;  // 0..1
    float cum = 0;
    for (uint8_t i = 0; i < n; i++) {
      cum += arr[i];
      if (r <= cum) return i;
    }
    return n - 1;
  }
};
