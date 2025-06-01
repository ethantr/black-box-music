#include "config.h"
#include "midi_helpers.h"
#include "chords.h"
#include "ultrasonic.h"
#include "VS1053MidiInterface.h"
#include "Sequencer.h"
#include "Step.h"
#include "Note.h"

UltrasonicSensor sensor1(TRIG_PIN, ECHO_PIN, "Sensor 1");
UltrasonicSensor sensor2(TRIG_PIN2, ECHO_PIN2, "Sensor 2");

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
  sensor2.begin();
  pinMode(PRESSURE_PIN, INPUT);

 uint16_t dur = 120;  // Short note
uint16_t gap = 80;   // Short pause

Note n0[] = { {60, 100, 0} }; // C
Note n1[] = { {63, 100, 0} }; // Eb
Note n2[] = { {67, 100, 0} }; // G
Note n3[] = { {44, 90, 0} };  // low Ab

Note n4[] = { {62, 100, 0} }; // D
Note n5[] = { {65, 100, 0} }; // F
Note n6[] = { {46, 90, 0} };  // low Bb

Note n7[] = { {68, 100, 0} }; // Ab
Note n8[] = { {48, 90, 0} };  // low C

Step steps[] = {
  Step(n0, 1, dur, gap), Step(n1, 1, dur, gap),
  Step(n2, 1, dur, gap), Step(n3, 1, dur, gap),

  Step(n4, 1, dur, gap), Step(n5, 1, dur, gap),
  Step(n2, 1, dur, gap), Step(n6, 1, dur, gap),

  Step(n1, 1, dur, gap), Step(n2, 1, dur, gap),
  Step(n7, 1, dur, gap), Step(n8, 1, dur, gap),

  Step(n0, 1, dur, gap), Step(n4, 1, dur, gap),
  Step(n5, 1, dur, gap), Step(n7, 1, dur, gap)
};
for (int i = 0; i < 16; i++) {
  sequencer.addStep(steps[i]);
}
sequencer.reset();

}


void loop() {

  sequencer.update();

  long distance = sensor2.getDistanceCM();

 
}
