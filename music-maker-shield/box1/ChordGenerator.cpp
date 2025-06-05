#include "ChordGenerator.h"

/// Define the three chord voicings (MIDI pitches):
///   chord 0 = Ab chord:  C(60), Eb(63), G(67),  low Ab(44)
///   chord 1 = Bb chord:  D(62), F(65), G(67),  low Bb(46)
///   chord 2 = C chord:   Eb(63), G(67), Ab(68), low C(48)
const uint8_t ChordGenerator::chordVoices[CHORD_COUNT][NOTES_PER_CHORD] = {
  { 60, 63, 67, 44 },
  { 62, 65, 67, 46 },
  { 63, 67, 68, 48 },
  { 65, 68, 72, 49 },  // chord 3 = Dbmaj7
  { 66, 70, 73, 51 },  // chord 4 = Ebm9
  { 70, 72, 75, 53 }   // chord 5 = F7sus4
};


/// Base Markov transition matrix (each row sums to ~1.0):
///   e.g., from chord 0, we might go to 0 with 0.2, 1 with 0.4, 2 with 0.4, etc.
///   These values are just an example—you can tweak for more or less “stickiness.”
const float ChordGenerator::BASE_TRANS[ChordGenerator::CHORD_COUNT][ChordGenerator::CHORD_COUNT] = {
  { 0.2, 0.2, 0.2, 0.2, 0.1, 0.1 },  // from chord 0
  { 0.15, 0.3, 0.2, 0.15, 0.1, 0.1 }, // from chord 1
  { 0.1, 0.2, 0.3, 0.2, 0.1, 0.1 },  // from chord 2
  { 0.1, 0.1, 0.2, 0.3, 0.15, 0.15 }, // from chord 3
  { 0.2, 0.15, 0.15, 0.1, 0.3, 0.1 }, // from chord 4
  { 0.1, 0.2, 0.15, 0.1, 0.15, 0.3 }  // from chord 5
};

