#include "config.h"
#include "lfo.h"
#include "midi_helpers.h"
#include "chords.h"
#include "ultrasonic.h"
#include "VS1053MidiInterface.h"
#include "Sequencer.h"
#include "Step.h"
#include "Note.h"

UltrasonicSensor sensor1(TRIG_PIN, ECHO_PIN, "Sensor 1");

VS1053MidiInterface midiInterface;
Sequencer sequencer(120, &midiInterface);

void setup() {
  Serial.begin(9600);
  VS1053_MIDI.begin(31250);

  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);

  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, 103);
  midiSetChannelVolume(0, MASTER_VOLUME);

  pinMode(LED_PIN, OUTPUT);
  sensor1.begin();
  pinMode(PRESSURE_PIN, INPUT);

  // Chord 1: Ab (C Eb G) with low Ab
  Note chord1[] = { {60, 100, 0}, {63, 100, 0}, {67, 100, 0}, {44, 90, 0} }; // + low Ab
Step step1(chord1, 4, 3000, 400);

// Chord 2: Bb (D F G) with low Bb
Note chord2[] = { {62, 100, 0}, {65, 100, 0}, {67, 100, 0}, {46, 90, 0} }; // + low Bb
Step step2(chord2, 4, 3000, 400);

// Chord 3: C (Eb G Ab) with low C
Note chord3[] = { {63, 100, 0}, {67, 100, 0}, {68, 100, 0}, {48, 90, 0} }; // + low C
Step step3(chord3, 4, 3000, 400);

// Add to sequencer
sequencer.addStep(step1);
sequencer.addStep(step2);
sequencer.addStep(step3);
sequencer.reset();

}


void loop() {

  sequencer.update();

  long pressure = analogRead(PRESSURE_PIN);
  Serial.println(pressure);
  uint8_t ccValue = map(pressure, 0, 1023, 0, 127);

  Serial.println(ccValue);

  // Send control changes on channel 0 (pad/chords)
  midiCC(0, 11, ccValue);  // Expression
  midiCC(0, 74, ccValue);  // Brightness
  midiCC(0, 1, ccValue);  // Reverb amount

 
}
