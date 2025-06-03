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

  midiSetChannelBank(0, VS1053_BANK_DRUMS1);
  midiSetInstrument(0, 56);
  midiSetChannelVolume(0, MASTER_VOLUME);

  pinMode(LED_PIN, OUTPUT);
  sensor1.begin();
  sensor2.begin();
  pinMode(PRESSURE_PIN, INPUT);
uint16_t drum_dur = 300;  // Very short note duration
uint16_t drum_gap = 80;  // Short gap between notes

// Basic drum sounds
Note kick[] = { {36, 100, 0} };        // Acoustic Bass Drum
Note snare[] = { {38, 100, 0} };       // Acoustic Snare
Note closed_hihat[] = { {42, 80, 0} };  // Closed Hi-Hat
Note open_hihat[] = { {46, 85, 0} };    // Open Hi-Hat

// Standard rock drum pattern (16 steps)
// Pattern: K-H-S-H-K-K-S-H-K-H-S-H-K-H-S-OH
Step steps[] = {
  Step(kick, 1, drum_dur, drum_gap),        // Beat 1
  Step(closed_hihat, 1, drum_dur, drum_gap), // &
  Step(snare, 1, drum_dur, drum_gap),       // Beat 2
  Step(closed_hihat, 1, drum_dur, drum_gap), // &
  
  Step(kick, 1, drum_dur, drum_gap),        // Beat 3
  Step(kick, 1, drum_dur, drum_gap),        // & (double kick)
  Step(snare, 1, drum_dur, drum_gap),       // Beat 4
  Step(closed_hihat, 1, drum_dur, drum_gap), // &
  
  Step(kick, 1, drum_dur, drum_gap),        // Beat 1
  Step(closed_hihat, 1, drum_dur, drum_gap), // &
  Step(snare, 1, drum_dur, drum_gap),       // Beat 2
  Step(closed_hihat, 1, drum_dur, drum_gap), // &
  
  Step(kick, 1, drum_dur, drum_gap),        // Beat 3
  Step(closed_hihat, 1, drum_dur, drum_gap), // &
  Step(snare, 1, drum_dur, drum_gap),       // Beat 4
  Step(open_hihat, 1, drum_dur, drum_gap)   // & (open hi-hat accent)
};
for (int i = 0; i < 16; i++) {
  sequencer.addStep(steps[i]);
}
sequencer.reset();

}


void loop() {

  long distance = sensor1.getDistanceCM();
  long distance2 = sensor2.getDistanceCM();
  
  long modDistance = map(distance, 5, 50, 100, 300);
  int dynamicVelocity = map(distance2, 5, 50, 100, 40);             // Shorter = louder
  dynamicVelocity = constrain(dynamicVelocity, 40, 127);
  long ledBrightness = map(distance, 5, 50, 255, 100);
  ledBrightness = constrain(ledBrightness, 0, 255);

  Serial.print("Brightness: "); 
  Serial.println(ledBrightness);
  
    for (int i = 0; i < 16; i++) { 
      if (distance < 50) {
        sequencer.setDuration(modDistance, i);
      } else { 
        sequencer.setDuration(300, i);
      }
      sequencer.setVelocity(dynamicVelocity, i);
  }

  analogWrite(LED_PIN,ledBrightness);
  sequencer.update();

  //delay(2000);
}
