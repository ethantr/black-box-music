#include "config.h"             // Your pin definitions, MASTER_VOLUME, etc.
#include "midi_helpers.h"
#include "PressureSensor.h"
#include "VS1053MidiInterface.h"
#include "ChordGenerator.h"      // Use chord generator to pick notes

// Two pressure sensors:
//   • sensor1 controls note intensity (velocity).
//   • sensor2 triggers generation/regen of melody.
PressureSensor sensor1 = PressureSensor(PRESSURE_PIN,   "Pressure 1");
PressureSensor sensor2 = PressureSensor(PRESSURE_PIN_2, "Pressure 2");

// One shared MIDI interface (the VS1053) on channel 1
VS1053MidiInterface midiInterface;

// Chord generator for progression and note choices
ChordGenerator chordGen;

// MIDI channel for melody
const int CHANNEL = 1;

//───────────────────────────────────────────────────────────────────────────────
// ** REAL-TIME NOTE-TRIGGER STATE **
unsigned long lastTouchTime = 0;                // last time sensor 2 was “active”
bool        isIdle       = false;               // are we currently in the “quiet” state?
const unsigned long IDLE_TIMEOUT = 5000;        // 5 000 ms of no touch → idle

// To debounce sensor 2 and avoid cutting notes too close together:
const unsigned long NOTE_DEBOUNCE_MS = 200;     // require 200 ms between triggers
unsigned long lastNoteTrigger = 0;

// Remember which pitch is currently “on” so we can turn it off later:
int  activePitch   = -1;                        
long noteOffTime   = 0;                         

void setup() {
  Serial.begin(9600);

  // Initialise VS1053 MIDI at 31250 baud
  VS1053_MIDI.begin(31250);
  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);

  // LED to indicate idle (optional)
  pinMode(LED_PIN, OUTPUT);

  // Initialise both sensors
  sensor1.begin();
  sensor2.begin();

  // Initialise chord generator and build an 8-step progression
  chordGen.begin();
  chordGen.regenerate(false);

  // Setup Channel 1 for “Lead 1 (Square)” = 104
  midiInterface.setBank(CHANNEL, VS1053_BANK_MELODY);
  midiSetInstrument(CHANNEL, 104);
  midiInterface.setVolume(CHANNEL, MASTER_VOLUME);

  // Prime the idle timer so we start “active”
  lastTouchTime = millis();
}

void loop() {
  unsigned long now = millis();

  //────────────────────────────────────────────────────────────────────────────
  // 1) HANDLE IDLE DETECTION (sensor 2 drives idle state)
  long raw2 = sensor2.read();
  if (raw2 > 40) {
    // We consider any reading > 40 as “touch.”
    lastTouchTime = now;
    if (isIdle) {
      isIdle = false;
      midiInterface.setVolume(CHANNEL, MASTER_VOLUME);
      // Regenerate a fresh chord progression whenever we “wake up.”
      chordGen.regenerate(false);
    }
  } 
  else if (!isIdle && (now - lastTouchTime > IDLE_TIMEOUT)) {
    // Went idle: fade out and silence everything
    isIdle = true;
    goQuietMelody();
  }
  //────────────────────────────────────────────────────────────────────────────

  // 2) IF NOT IDLE, CHECK FOR NEW NOTE TRIGGER ON sensor 2
  if (!isIdle) {
    // Debounce so we don’t retrigger too fast:
    if (raw2 > 700 && (now - lastNoteTrigger > NOTE_DEBOUNCE_MS)) {
      triggerNextNote();
      lastNoteTrigger = now;
      lastTouchTime   = now;                   // also keeps us out of idle
    }

    // 3) TURN‐OFF any note that needs silencing
    if (activePitch >= 0 && now >= noteOffTime) {
      midiNoteOff(CHANNEL, activePitch, 0);
      activePitch = -1;
    }
  }

  // 4) FEEDBACK (optional)
  Serial.print("VelSensor raw1 = ");
  Serial.print(sensor1.read());
  Serial.print("   GenSensor raw2 = ");
  Serial.println(raw2);
}


//───────────────────────────────────────────────────────────────────────────────
// Play exactly one short note each time sensor 2 is pressed. Velocity is read
// from sensor 1 at the instant of trigger. Pitch is one of the top 3 voices
// of the next chord in the progression. The note will be turned off 200 ms later.

void triggerNextNote() {
  // 1) Read raw1 → velocity (60..127 range)
  long raw1 = sensor1.read();
  uint8_t vel = map(raw1, 0, 1023, 60, 127);

  // 2) Advance chord progression based on raw2 (pressure) bias:
  uint8_t chordIdx = chordGen.nextChord(sensor2.read());

  // 3) Pick a random chord‐tone among the top 3 voices
  uint8_t toneIdx  = random(3);  
  uint8_t basePitch = ChordGenerator::chordVoices[chordIdx][toneIdx];

  // 4) Optionally transpose to taste (e.g. +12 for one octave up):
  uint8_t pitch = basePitch + 12; 

  // 5) Send noteOn immediately:
  midiNoteOn(CHANNEL, pitch, vel);
  activePitch = pitch;
  noteOffTime = millis() + 200;   // hold for 200 ms then turn off
}


//───────────────────────────────────────────────────────────────────────────────
// Gradually fade Channel 1 from MASTER_VOLUME → 0, then clear activePitch

void goQuietMelody() {
  const uint8_t stepSize  = 5;      // volume decrement per step (5/127)
  const uint16_t stepDelay = 50;    // ms delay between steps to make fade faster

  for (int vol = MASTER_VOLUME; vol > 0; vol -= stepSize) {
    midiInterface.setVolume(CHANNEL, vol);
    delay(stepDelay);
  }
  midiInterface.setVolume(CHANNEL, 0);

  // If a note is still on, turn it off immediately:
  if (activePitch >= 0) {
    midiNoteOff(CHANNEL, activePitch, 0);
    activePitch = -1;
  }
}
