#include "config.h"
#include "midi_helpers.h"
#include "chords.h"
#include "ultrasonic.h"
#include "VS1053MidiInterface.h"
#include "Sequencer.h"
#include "Step.h"
#include "Note.h"
#include "lfo.h"
#include "ChordGenerator.h"



// Two ultrasonic sensors
UltrasonicSensor sensor1(TRIG_PIN, ECHO_PIN, "Distance 1");
UltrasonicSensor sensor2(TRIG_PIN2, ECHO_PIN2, "Distance 2");

// MIDI interface
VS1053MidiInterface midiInterface;

// Single sequencer for pad/chords
Sequencer sequencer(90, &midiInterface);   // Pad sequencer on channel 0

ChordGenerator chordGen;
LFO brightnessLFO(0.2, 20, 80);    // LFO for filter effects

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

  // Pad sound on Channel 0
  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, 90);  // Pad 1 (New Age)
  midiSetChannelVolume(0, MASTER_VOLUME);

  // LFO initialization
  brightnessLFO.begin();

  // Build initial 8-step chord progression
  chordGen.regenerate(false);
  loadChordsIntoSequencer();
  
  Serial.println("Pad Box with Distance Sensors Ready!");
}

void loop() {
  unsigned long now = millis();

  sequencer.update();
  // Read sensor distances (normalized: 0.0 = far, 1.0 = close)
  long dist1 = sensor1.getDistanceCM();
  long dist2 = sensor2.getDistanceCM();
  //Serial.print("Distance 1: "); 
  //Serial.println(dist1);
  Serial.print("Distance 2: "); 
  Serial.println(dist2); 

  float dist1Norm = getNormalized(80.0, dist1); //80cm max range
  float dist2Norm = getNormalized(60.0, dist2); //60cm max range


  /*
  long ledBrightness = map(dist1, 5, 50, 255, 100);
  ledBrightness = constrain(ledBrightness, 0, 255);
  long ledBrightness2 = map(dist2, 5, 50, 255, 100);
  ledBrightness2 = constrain(ledBrightness2, 0, 255);

  if (dist1 < 50) { 
    analogWrite(LED_PIN, ledBrightness); 
  } if (dist2 < 50) { 
    analogWrite(LED_PIN, ledBrightness2);
  } if (dist1 > 50 && dist2 > 50) { 
    digitalWrite(LED_PIN, LOW);
  }*/

  // Sensor 1: Controls chord regeneration rate
  // Closer = faster chord changes
  unsigned long regenInterval = map(dist1Norm * 1000, 0, 1000, 8000, 1000);
  if (now - lastChordTime > regenInterval && dist1Norm > 0.1) {
    bool biasRepeat = (dist1Norm > 0.7);
    chordGen.regenerate(biasRepeat);
    loadChordsIntoSequencer();
    regeneratingFeedback();
    lastChordTime = now;
  }

  // Sensor 2: Controls volume
  // Closer = louder
  uint8_t volume = map(dist2Norm * 127, 0, 127, 20, 100);
  midiCC(0, 7, volume);  // CC 7: Volume

    // LED shows activity from either sensor
  if (dist1Norm > 0.3 || dist2Norm > 0.3) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

float getNormalized(float maxRange, float dist) {
  return constrain(1.0 - (dist / maxRange), 0.0, 1.0);
}

// Load all 8 generated chords into sequencer
void loadChordsIntoSequencer() {
  sequencer.clear();
  for (uint8_t i = 0; i < ChordGenerator::PROG_LENGTH; i++) {
    uint8_t cIdx = chordGen.progression[i];
    // Build a 4-note voicing: root, third, fifth, low bass
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

// Helper: blink LED quickly to show chord regeneration
void regeneratingFeedback() {
  digitalWrite(LED_PIN, HIGH);
  delay(80);
  digitalWrite(LED_PIN, LOW);
}