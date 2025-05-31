#include "config.h"
#include "lfo.h"
#include "midi_helpers.h"
#include "chords.h"
#include "ultrasonic.h"

UltrasonicSensor sensor1(TRIG_PIN, ECHO_PIN, "Sensor 1");

void setup() {
  Serial.begin(9600);
  VS1053_MIDI.begin(31250);

  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);

  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, VS1053_GM1_ECHOES);
  midiSetChannelVolume(0, 100);

  pinMode(LED_PIN, OUTPUT);
  sensor1.begin();
  pinMode(PRESSURE_PIN, INPUT);
}

void loop() {
  unsigned long now = millis();
  unsigned long dt = now - lastLFO;

  if (dt > 20) {
    midiCC(1, 7, updateLFO(volLFO, dt));
    lastLFO = now;
  }

  long dist = 2;
  int pressure = analogRead(PRESSURE_PIN);
  Serial.print("[PRESURE] ");
  Serial.println(pressure);

  uint8_t cIdx = map(pressure, 0, 1023, 0, NUM_CHORDS - 1);
  const uint8_t* chord = chords[cIdx];

  float depth = map(pressure, 0, 1023, 10, 50);
  velLFO.minVal = BASE_VEL - depth;
  velLFO.maxVal = BASE_VEL + depth;

  midiSetInstrument(0, pressure < 512 ? VS1053_GM1_OCARINA : VS1053_GM1_ECHOES);

  unsigned long interval = map(dist, 5, 100, 200, 800);
  int dynVel = constrain(map(dist, 5, 100, 100, 50), 10, 127);

  if (now - lastMel > interval) {
    uint8_t note = scaleNotes[chord[random(0, 3)]];
    midiNoteOn(0, note, dynVel);
    digitalWrite(LED_PIN, HIGH);
    delay(30);
    midiNoteOff(0, note, 0);

    for (uint8_t i = 0; i < 3; i++) {
      uint8_t root = scaleNotes[chord[i]] - 12;
      midiNoteOn(1, root, dynVel / 2);
    }
    delay(100);
    for (uint8_t i = 0; i < 3; i++) {
      uint8_t root = scaleNotes[chord[i]] - 12;
      midiNoteOff(1, root, 0);
    }

    digitalWrite(LED_PIN, LOW);
    lastMel = now;
  }
}
