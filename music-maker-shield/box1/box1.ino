#include "config.h"           // Your pin definitions, MASTER_VOLUME, etc.
#include "midi_helpers.h"
#include "PressureSensor.h"
#include "VS1053MidiInterface.h"
#include "Sequencer.h"
#include "Step.h"
#include "Note.h"

#include "ChordGenerator.h"
#include "LFO.h"

// Two pressure sensors:
PressureSensor sensor1 = PressureSensor(PRESSURE_PIN, "Pressure 1");
PressureSensor sensor2 = PressureSensor(PRESSURE_PIN_2, "Pressure 2");

// One shared MIDI interface (the VS1053) on channels 0 and 1
VS1053MidiInterface midiInterface;

// Two independent Sequencers
Sequencer sequencer(90, &midiInterface);   // Box A: pads/chords (slower BPM)
Sequencer sequencer2(120, &midiInterface); // Box B: melody (faster BPM)

ChordGenerator chordGen;
LFO         brightnessLFO(0.2, 20, 80);    // Initial slow LFO (0.2 Hz, range 20–80)

int currentMelodyStep = 0;
unsigned long lastChordTime = 0;

void setup() {
  Serial.begin(9600);

  // Initialize hardware & sensors
  VS1053_MIDI.begin(31250);
  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);
  pinMode(LED_PIN, OUTPUT);
  sensor1.begin();
  sensor2.begin();

  // ChordGenerator setup
  chordGen.begin();

  // Sequencer A (Chord/Pad) → Channel 0, “Pad 1 (New Age)” = 88
  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, 90);
  midiSetChannelVolume(0, MASTER_VOLUME);

  // Sequencer B (Melody/Lead) → Channel 1, “Flute” = 73
  midiSetChannelBank(1, VS1053_BANK_MELODY);
  midiSetInstrument(1, 104);
  midiSetChannelVolume(1, MASTER_VOLUME);

  // LFO initialisation (for CC 74 on Channel 0)
  brightnessLFO.begin();

  // Build an initial 8‐step chord progression (no bias)
  chordGen.regenerate(false);
  loadChordsIntoSequencer();

  // Build an initial 8‐step melody that matches the chords
  generateMelodySteps();
}

void loop() {
  unsigned long now = millis();

  // 1) Update Sequencers:
  sequencer.update();
  sequencer2.update();

  // 2) Read sensor1, map to normalized 0..1
  long raw1 = sensor1.read();               
  float p1Norm = constrain(raw1 / 1023.0, 0.0, 1.0);

  // 3) Regenerate chords at a rate inversely proportional to pressure:
  unsigned long regenInterval = map(raw1, 0, 1023, 8000, 1000);
  if (now - lastChordTime > regenInterval) {
    bool biasRepeat = (p1Norm > 0.7);
    chordGen.regenerate(biasRepeat);
    loadChordsIntoSequencer();
    regeneratingFeedback();
    lastChordTime = now;
  }

  // 4) Drive an LFO for filter cutoff (CC 74) and resonance (CC 71) on Channel 0:
  //
  //    - LFO frequency: 0.05 Hz (very slow) → 2.0 Hz (fast) as pressure climbs.  
  //    - LFO depth (min..max): (40..100) → (80..127) as pressure climbs (so resonance is more obvious).
  float lfoFreq = 0.05 + p1Norm * 1.95;       // 0.05 Hz … 2.0 Hz
  uint8_t lfoMin  = 40  + (uint8_t)(p1Norm * 40);  // e.g. 40 … 80
  uint8_t lfoMax  = 100 + (uint8_t)(p1Norm * 27);  // e.g. 100 … 127

  brightnessLFO.setFreq(lfoFreq);
  brightnessLFO.setDepth(lfoMin, lfoMax);
  uint8_t ccVal = brightnessLFO.update();

  // Send both CC 74 (cutoff) and CC 71 (resonance) on Channel 0:
  midiCC(0, 74, ccVal);                    // CC 74: Filter Cutoff
  // For resonance, we map ccVal into a higher baseline so the “peak” is more obvious:
  uint8_t ccRes = map(ccVal, 0, 127, lfoMin, 127);
  midiCC(0, 71, ccRes);                    // CC 71: Filter Resonance (Q)

  // 5) Read sensor2 for melody interactions:
  long raw2 = sensor2.read();               
  float p2Norm = constrain(raw2 / 1023.0, 0.0, 1.0);

  // 5a) Map sensor2 to “modulation” CC on Channel 1 (vibrato)
  uint8_t ccMod = map(raw2, 0, 1023, 0, 127);
  midiCC(1, 1, ccMod);   // CC 1: Modulation Wheel

  // 5b) Map sensor2 to “reverb” CC on Channel 1
  uint8_t ccRev = map(raw2, 0, 1023, 20, 100);
  midiCC(1, 91, ccRev);  // CC 91: Reverb Send

  // 6) If sensor2 is pressed above a threshold, insert a staccato flourish
  static unsigned long lastInsert = 0;
  if (raw2 > 700 && now - lastInsert > 500) {
    insertStaccatoFlourish(currentMelodyStep, p2Norm);
    currentMelodyStep = (currentMelodyStep + 1) % 16;
    lastInsert = now;
  }
}

// Helper: blink LED quickly to show chord regeneration
void regeneratingFeedback() {
  digitalWrite(LED_PIN, HIGH);
  delay(80);
  digitalWrite(LED_PIN, LOW);
}

// Load all 8 generated chords into sequencer (Box A)
void loadChordsIntoSequencer() {
  sequencer.clear();
  for (uint8_t i = 0; i < ChordGenerator::PROG_LENGTH; i++) {
    uint8_t cIdx = chordGen.progression[i];
    // Build a 4‐note voicing: root, third, fifth, low bass:
    Note notes[4] = {
      { ChordGenerator::chordVoices[cIdx][0], 80, 0 },
      { ChordGenerator::chordVoices[cIdx][1], 75, 0 },
      { ChordGenerator::chordVoices[cIdx][2], 75, 0 },
      { ChordGenerator::chordVoices[cIdx][3], 65, 0 }
    };
    Step s(notes, 4, 3000, 200);
    sequencer.addStep(s);
  }
  sequencer.reset();
}

// Generate the initial 16‐step melody (Box B) based on the current progression
void generateMelodySteps() {
  sequencer2.clear();
  for (uint8_t i = 0; i < 16; i++) {
    uint8_t chordIdx = chordGen.progression[i % ChordGenerator::PROG_LENGTH];
    // pick a random chord tone among the top 3 (ignore the low bass at index 3)
    uint8_t toneIndex = random(3);
    uint8_t pitch = ChordGenerator::chordVoices[chordIdx][toneIndex] + 12; 

    // Wrap the three values in braces for the single Note array element:
    Note mel[1] = { { pitch, 80, 1 } };

    Step s(mel, 1, 400, 100);
    sequencer2.addStep(s);
  }
  sequencer2.reset();
}


// Insert a quick staccato flourish two octaves above the chosen chord tone
void insertStaccatoFlourish(int idx, float p2Norm) {
  uint8_t chordIdx = chordGen.progression[idx % ChordGenerator::PROG_LENGTH];
  uint8_t toneIdx = random(3);
  uint8_t pitch = ChordGenerator::chordVoices[chordIdx][toneIdx] + 24;
  uint8_t vel = map((int)(p2Norm * 1023), 0, 1023, 60, 127);

  // Again, wrap the values in braces for the single Note object:
  Note mel[1] = { { pitch, vel, 1 } };

  uint16_t dur   = 200;                                // very short
  uint16_t delay = map((int)(p2Norm * 1023), 0, 1023, 200, 50);
  Step s(mel, 1, dur, delay);
  sequencer2.setStep(idx, s);
}

