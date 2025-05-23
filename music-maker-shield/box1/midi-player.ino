//***************************************************
//  Pressure-Controlled VS1053 MIDI Sketch
//  Uses ultrasonic distance for tempo/velocity and pressure sensor
//  (A3) to select chords and modulate expression.
//  Written by Ethan, incorporating Adafruit example.
//***************************************************

// Pin definitions
#define VS1053_RX      2   // connects to VS1053 RX
#define VS1053_RESET   9   // connects to VS1053 RESET (if needed)
#define TRIG_PIN       3   // ultrasonic trigger pin
#define ECHO_PIN       4   // ultrasonic echo pin
#define LED_PIN        5   // onboard LED
#define PRESSURE_PIN   A3  // pressure sensor input

// MIDI and Adafruit constants
#define VS1053_BANK_MELODY  0x79
#define VS1053_GM1_OCARINA   80
#define VS1053_GM1_ECHOES   100
#define MIDI_NOTE_ON       0x90
#define MIDI_NOTE_OFF      0x80
#define MIDI_CC            0xB0
#define MIDI_CHAN_MSG      0xB0
#define MIDI_CHAN_BANK     0x00
#define MIDI_CHAN_VOLUME   0x07
#define MIDI_CHAN_PROGRAM  0xC0

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
#include <SoftwareSerial.h>
SoftwareSerial VS1053_MIDI(0, VS1053_RX); // TX only
#else
#define VS1053_MIDI Serial1
#endif

#include <Arduino.h>
#include <SPI.h>
#include <math.h>

// Expression and timing constants
const uint8_t BASE_VEL = 70;  // centre velocity
unsigned long lastLFO = 0;
unsigned long lastMel = 0;

// LFO definition
struct LFO { float phase, freq; uint8_t minVal, maxVal; };
LFO velLFO = {0.0, 0.1, BASE_VEL-20, BASE_VEL+20};
LFO volLFO = {0.5, 0.05, 40,100};

uint8_t updateLFO(LFO& lfo, unsigned long dt) {
  lfo.phase += lfo.freq * (dt/1000.0);
  if (lfo.phase >= 1.0) lfo.phase -= 1.0;
  float s = sin(2*PI*lfo.phase);
  float mid = (lfo.maxVal + lfo.minVal)/2.0;
  float amp = (lfo.maxVal - lfo.minVal)/2.0;
  return constrain((int)(mid + amp*s),0,127);
}

void midiCC(uint8_t ch, uint8_t cc, uint8_t val) {
  if(ch>15||cc>127||val>127) return;
  VS1053_MIDI.write(MIDI_CC|ch);
  VS1053_MIDI.write(cc);
  VS1053_MIDI.write(val);
}

// Scale definition (C major) and chords
// const uint8_t scaleNotes[] = {60,62,64,65,67,69,71,72};
// const uint8_t SCALE_LEN = sizeof(scaleNotes);
// // Define triads by scale indices: I, IV, V, vi
// const uint8_t chords[][3] = {
//   {0,2,4}, // C major
//   {3,5,0}, // F major
//   {4,6,1}, // G major
//   {5,7,2}  // A minor
// };
// const uint8_t NUM_CHORDS = sizeof(chords)/sizeof(chords[0]);

const uint8_t scaleNotes[] = {69,70,71,72,73,74,75,76,77,78,79,80};  // A4 to F#5
const uint8_t SCALE_LEN = sizeof(scaleNotes);
// Define minor triads by chromatic indices relative to A4: root, minor third (+3), perfect fifth (+7)
const uint8_t chords[][3] = {
  {0,3,7},   // A minor (A,C,E)
  {2,5,9},   // B minor (B,D,F#)
  {4,7,11},  // C# minor (C#,E,G#)
  {5,8,0},   // D minor (D,F,A)
  {7,10,2},  // E minor (E,G,B)
  {9,0,4},   // F# minor (F#,A,C#)
  {11,2,5}   // G# minor (G#,B,D#)
};
const uint8_t NUM_CHORDS = sizeof(chords)/sizeof(chords[0]);

long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long d = pulseIn(ECHO_PIN, HIGH);
  return constrain((d*0.0343)/2,5,100);
}

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
  midiSetChannelVolume(0,100);

  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(PRESSURE_PIN, INPUT);
}

void loop() {
  unsigned long now = millis();
  unsigned long dt = now - lastLFO;
  if (dt > 20) {
    midiCC(1,7, updateLFO(volLFO, dt));
    lastLFO = now;
  }

  long dist = getDistanceCM();
  int pressure = analogRead(PRESSURE_PIN);

  // Select chord by pressure
  uint8_t cIdx = map(pressure,0,1023,0,NUM_CHORDS-1);
  const uint8_t* chord = chords[cIdx];

  // Modulate LFO depth by pressure
  float depth = map(pressure,0,1023,10,50);
  velLFO.minVal = BASE_VEL - depth;
  velLFO.maxVal = BASE_VEL + depth;

  // Alternate instruments by pressure
  midiSetInstrument(0, pressure<512 ? VS1053_GM1_OCARINA : VS1053_GM1_ECHOES);

  // Tempo & velocity from distance
  unsigned long interval = map(dist,5,100,200,800);
  int dynVel = constrain(map(dist,5,100,100,50),10,127);

  if (now - lastMel > interval) {
    // Melody: pick random note from chord
    uint8_t note = scaleNotes[chord[random(0,3)]];
    midiNoteOn(0,note,dynVel);
    digitalWrite(LED_PIN, HIGH);
    delay(30);
    midiNoteOff(0,note,0);
    
    

    // Arpeggiate chord on channel 1
    for(uint8_t i=0;i<3;i++) {
      uint8_t root = scaleNotes[chord[i]] - 12; // lower octave
      midiNoteOn(1, root, dynVel/2);
    }
    delay(100);
    for(uint8_t i=0;i<3;i++) {
      uint8_t root = scaleNotes[chord[i]] - 12;
      midiNoteOff(1, root, 0);
      // 
    }
    digitalWrite(LED_PIN, LOW);
    lastMel = now;
  }
}

//--- MIDI helpers ---
void midiSetInstrument(uint8_t ch, uint8_t inst){
  if(ch>15) return;
  inst--;
  if(inst>127) return;
  VS1053_MIDI.write(MIDI_CHAN_PROGRAM|ch);
  VS1053_MIDI.write(inst);
}

void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}
