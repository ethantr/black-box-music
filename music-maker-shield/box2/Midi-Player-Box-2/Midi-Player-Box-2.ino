// ──────────────────────────────────────────────────────────────────────────────
// Any includes + globals remain the same as before:

#include "config.h"
#include "midi_helpers.h"
#include "Ultrasonic.h"
#include "VS1053MidiInterface.h"
#include "Sequencer.h"
#include "Step.h"
#include "Note.h"
#include "LFO.h"
#include "ChordGenerator.h"

// Two ultrasonic sensors
UltrasonicSensor sensor1(TRIG_PIN,  ECHO_PIN,  "Distance 1");
UltrasonicSensor sensor2(TRIG_PIN2, ECHO_PIN2, "Distance 2");

// MIDI interface + sequencer
VS1053MidiInterface midiInterface;
Sequencer sequencer(90, &midiInterface);

// Chord generator + LFO
ChordGenerator chordGen;
LFO brightnessLFO(0.2, 20, 80);

// Idle‐detection state
unsigned long lastChordTime  = 0;
bool         wasSomeoneClose = false;
float        closeThreshold  = 0.3f;
float        farThreshold    = 0.25f;

// ──────────────────────────────────────────────────────────────────────────────
// (Include the helper function here)

float readAverageDistance(UltrasonicSensor& us,
                          int samples,
                          float minCm,
                          float maxCm) 
{
  long sum = 0;
  int  validCount = 0;

  for (int i = 0; i < samples; i++) { 
    long d = us.getDistanceCM();
    if (d >= minCm && d <= maxCm) {
      sum += d;
      validCount++;
    }
    delay(5);
  }

  if (validCount == 0) {
    return -1.0f;
  }
  return (float)sum / validCount;
}

// Normalize a distance (cm) to 0.0..1.0 “closeness”
// (0.0 if out of range, 1.0 if at zero cm)
float getNormalized(float maxRange, float distCM) {
  if (distCM < 0) {
    return 0.0f;
  }
  float n = 1.0f - (distCM / maxRange);
  return constrain(n, 0.0f, 1.0f);
}

// ──────────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(9600);

  // initialise VS1053 MIDI
  VS1053_MIDI.begin(31250);
  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);

  pinMode(LED_PIN, OUTPUT);

  // initialise ultrasonic sensors
  sensor1.begin();
  sensor2.begin();

  // setup ChordGenerator
  chordGen.begin();
  chordGen.regenerate(false);

  // configure sequencer (channel 0)
  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, 90);            // “Pad 1 (New Age)”
  midiSetChannelVolume(0, MASTER_VOLUME);

  brightnessLFO.begin();
  loadChordsIntoSequencer();
  Serial.println("Pad Box with Averaged Sensors Ready!");
}

void loop() {
  unsigned long now = millis();

  // 1) Read **average** distance for each sensor (5 samples each)
  float raw1  = readAverageDistance(sensor1, 5, 2.0f, 80.0f);  // chord‐trigger sensor
  float raw2  = readAverageDistance(sensor2, 5, 2.0f, 60.0f);  // volume/mod sensor

  // 2) Convert negative (–1) into “out of range” (very far)
  if (raw1 < 0) raw1 = 80.0f;   // treat as far beyond max
  if (raw2 < 0) raw2 = 60.0f;

  // 3) Normalise to 0..1 closeness
  float dist1Norm = getNormalized(80.0f, raw1);
  float dist2Norm = getNormalized(60.0f, raw2);

  // LED brightness from sensor 2 (closer = brighter)
  int brightness = map((int)(dist2Norm * 100), 0, 100, 0, 255);
  brightness = constrain(brightness, 0, 255);
  analogWrite(LED_PIN, brightness);

  // ────────────────────────────────────────────────────────────────────────────
  // 4) “Someone moved into range?” logic (sensor1)
  bool isSomeoneClose = (dist1Norm > closeThreshold);
  if (isSomeoneClose && !wasSomeoneClose) {
    // ── Regenerate progression as before:
    bool biasRepeat = (dist1Norm > 0.7f);
    chordGen.regenerate(biasRepeat);
    loadChordsIntoSequencer();
    regeneratingFeedback();
    sequencer.reset();
    lastChordTime = now;

    // ── **Play the first chord immediately** for a quick sound cue:
    {
      // First chord index is progression[0]:
      uint8_t firstIdx = chordGen.progression[0];
      // Play top 3 voices (ignore the low bass at index 3):
      for (uint8_t j = 0; j < 3; j++) {
        uint8_t pitch = ChordGenerator::chordVoices[firstIdx][j];
        midiInterface.noteOn(0, pitch, 100);  // channel 0, velocity 100
      }
      delay(100);  // hold chord for 100 ms
      for (uint8_t j = 0; j < 3; j++) {
        uint8_t pitch = ChordGenerator::chordVoices[firstIdx][j];
        midiInterface.noteOff(0, pitch, 0);
      }
    }
  }
  if (!isSomeoneClose && dist1Norm < farThreshold) {
    wasSomeoneClose = false;
  } else if (isSomeoneClose) {
    wasSomeoneClose = true;
  }

  // ────────────────────────────────────────────────────────────────────────────
  // 5) Update sequencer only if someone is close
  if (isSomeoneClose) {
    sequencer.update();
  }

  // If it's been more than 5 seconds since last chord and no one is close, stop all notes
  if (!isSomeoneClose && (now - lastChordTime > 5000)) {
    stopAllNotes();     
    sequencer.reset();  
  }

  // ────────────────────────────────────────────────────────────────────────────
  // 6) Map dist2Norm to MIDI Volume (CC7)
  int midiRange = (int)round(dist2Norm * 127.0f);
  midiRange = constrain(midiRange, 0, 127);
  uint8_t volumeCC = map(midiRange, 0, 127, 20, 100);
  midiCC(0, 7, volumeCC);   // CC 7 = Channel 0 Volume

  // 7) Optionally map dist2Norm to modulation (CC1)
  uint8_t modCC = map(midiRange, 0, 127, 0, 127);
  midiCC(0, 1, modCC);      

  // ────────────────────────────────────────────────────────────────────────────
  // 8) Debug output (optional)
  Serial.print("Avg Dist1(cm): ");
  Serial.print(raw1, 1);
  Serial.print("  Norm1: ");
  Serial.print(dist1Norm, 2);
  Serial.print("  Avg Dist2(cm): ");
  Serial.print(raw2, 1);
  Serial.print("  Norm2: ");
  Serial.println(dist2Norm, 2);
}

// ──────────────────────────────────────────────────────────────────────────────
// Load 8 chords into sequencer (unchanged from before):
void loadChordsIntoSequencer() {
  sequencer.clear();
  for (uint8_t i = 0; i < ChordGenerator::PROG_LENGTH; i++) {
    uint8_t cIdx = chordGen.progression[i];
    Note notes[4] = {
      { ChordGenerator::chordVoices[cIdx][0], 80, 0 },
      { ChordGenerator::chordVoices[cIdx][1], 75, 0 },
      { ChordGenerator::chordVoices[cIdx][2], 75, 0 },
      { ChordGenerator::chordVoices[cIdx][3], 65, 0 }
    };
    Step s(notes, 4, 3000, 200);
    sequencer.addStep(s);
  }
}

// Quick LED blink to show chord regeneration
void regeneratingFeedback() {
  // digitalWrite(LED_PIN, HIGH);
  // delay(80);
  // digitalWrite(LED_PIN, LOW);
}

void stopAllNotes() {
  for (int note = 0; note < 128; note++) {
    midiInterface.noteOff(0, note, 0);
  }
}
