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
Sequencer sequencer2(120, &midiInterface); 

void setup() {
  Serial.begin(9600);
  VS1053_MIDI.begin(31250);

  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);

  midiSetChannelBank(0, VS1053_BANK_DRUMS1);
  midiSetChannelBank(1, VS1053_BANK_MELODY); 
  midiSetInstrument(0, 56);
  midiSetInstrument(1, VS1053_GM1_ELECTRIC_GUITAR);
  midiSetChannelVolume(0, MASTER_VOLUME);

  pinMode(LED_PIN, OUTPUT);
  sensor1.begin();
  sensor2.begin();
  pinMode(PRESSURE_PIN, INPUT);
uint16_t drum_dur = 300;  // Very short note duration
uint16_t drum_gap = 80;  // Short gap between notes

// Slower, more melodic guitar riff settings
uint16_t guitar_dur = 400;    // Longer note duration for sustained feel
uint16_t guitar_gap = 200;    // Moderate gap for smooth flow

// Basic drum sounds
Note kick[] = { {36, 100, 0} };        // Acoustic Bass Drum
Note snare[] = { {38, 100, 0} };       // Acoustic Snare
Note closed_hihat[] = { {42, 80, 0} };  // Closed Hi-Hat
Note open_hihat[] = { {46, 85, 0} };    // Open Hi-Hat

// Guitar notes (channel 1) - focusing on a bluesy/rock progression
Note gtr_E[] = { {64, 90, 1} };     // E4 
Note gtr_G[] = { {67, 85, 1} };     // G4
Note gtr_A[] = { {69, 95, 1} };     // A4 - accent
Note gtr_B[] = { {71, 80, 1} };     // B4
Note gtr_C[] = { {72, 85, 1} };     // C5
Note gtr_D[] = { {74, 90, 1} };     // D5
Note gtr_F[] = { {65, 85, 1} };     // F4
Note gtr_low_E[] = { {52, 100, 1} }; // E3 - power chord root
Note gtr_low_A[] = { {57, 95, 1} };  // A3 - strong bass note
Note rest[] = { {0, 0, 1} };         // Rest/silence


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

Step melodic_pattern[] = {
  Step(gtr_low_E, 1, guitar_dur * 2, guitar_gap), // 1 - strong root (longer)
  Step(rest, 1, guitar_dur, guitar_gap),           // & - rest
  Step(gtr_G, 1, guitar_dur, guitar_gap),          // 2 - melody note
  Step(gtr_A, 1, guitar_dur, guitar_gap),          // & - accent
  
  Step(gtr_E, 1, guitar_dur * 1.5, guitar_gap),   // 3 - sustained
  Step(rest, 1, guitar_dur / 2, guitar_gap),       // & - short rest
  Step(gtr_D, 1, guitar_dur, guitar_gap),          // 4 - descending
  Step(gtr_C, 1, guitar_dur, guitar_gap),          // & - continue descent
  
  Step(gtr_low_A, 1, guitar_dur * 2, guitar_gap), // 1 - bass movement (longer)
  Step(rest, 1, guitar_dur, guitar_gap),           // & - rest
  Step(gtr_B, 1, guitar_dur, guitar_gap),          // 2 - melody
  Step(gtr_G, 1, guitar_dur, guitar_gap),          // & - harmony
  
  Step(gtr_E, 1, guitar_dur, guitar_gap),          // 3 - return to root
  Step(gtr_F, 1, guitar_dur, guitar_gap),          // & - passing tone
  Step(gtr_G, 1, guitar_dur, guitar_gap),          // 4 - resolution
  Step(gtr_low_E, 1, guitar_dur * 1.5, guitar_gap) // & - strong ending
};


for (int i = 0; i < 16; i++) {
  sequencer.addStep(steps[i]);
  //sequencer2.addStep(melodic_pattern[i]);
}
sequencer.reset();
sequencer2.reset();

}


void loop() {

  long distance = sensor1.getDistanceCM();
  long distance2 = sensor2.getDistanceCM();
  
  long dynamicDuration = map(distance, 5, 50, 100, 300);
  int dynamicVelocity = map(distance2, 5, 50, 100, 40);             // Shorter = louder
  dynamicVelocity = constrain(dynamicVelocity, 40, 127);
  long dynamicGap = map(distance2, 5, 50, 50, 200);
  long ledBrightness = map(distance, 5, 50, 255, 100);
  ledBrightness = constrain(ledBrightness, 0, 255);
  long ledBrightness2 = map(distance2, 5, 50, 255, 100);
  ledBrightness2 = constrain(ledBrightness2, 0, 255);

  Serial.print("Gap: "); 
  Serial.println(dynamicGap);
  
    for (int i = 0; i < 16; i++) { 
      if (distance < 50) {
        sequencer.setDuration(dynamicDuration, i);
        sequencer2.setGap(dynamicGap, i);
        analogWrite(LED_PIN,ledBrightness);
      } else { 
        sequencer.setDuration(300, i);
        sequencer2.setGap(200, i);
      }
      sequencer.setVelocity(dynamicVelocity, i);
      if (distance2 < 50) { 
        analogWrite(LED_PIN,ledBrightness2);
      }
      if (distance > 50 && distance2 > 50) { 
        digitalWrite(LED_PIN, LOW);
      }

  }
  sequencer.update();
  sequencer2.update();

  //delay(2000);
}
