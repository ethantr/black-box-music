/*************************************************** 
  This is an example for the Adafruit VS1053 Codec Breakout

  Designed specifically to work with the Adafruit VS1053 Codec Breakout 
  ----> https://www.adafruit.com/products/1381

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

// define the pins used
#define VS1053_RX  2 // This is the pin that connects to the RX pin on VS1053

#define VS1053_RESET 9 // This is the pin that connects to the RESET pin on VS1053
// If you have the Music Maker shield, you don't need to connect the RESET pin!

// If you're using the VS1053 breakout:
// Don't forget to connect the GPIO #0 to GROUND and GPIO #1 pin to 3.3V
// If you're using the Music Maker shield:
// Don't forget to connect the GPIO #1 pin to 3.3V and the RX pin to digital #2

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31
#define VS1053_BANK_DEFAULT 0x00
#define VS1053_BANK_DRUMS1 0x78
#define VS1053_BANK_DRUMS2 0x7F
#define VS1053_BANK_MELODY 0x79

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!
#define VS1053_GM1_OCARINA 80

#define VS1053_GM1_ECHOES 39

#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0

#if defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__)
  #include <SoftwareSerial.h>
  SoftwareSerial VS1053_MIDI(0, 2); // TX only, do not use the 'rx' side
#else
  // on a Mega/Leonardo you may have to change the pin to one that 
  // software serial support uses OR use a hardware serial port!
  #define VS1053_MIDI Serial1
#endif


// --- LFO definition ---

struct LFO {
  float phase;     // 0.0–1.0
  float freq;      // Hz
  uint8_t minVal;  // minimum output
  uint8_t maxVal;  // maximum output
};

LFO velLFO = { 0.0, 0.1,  -20,  20 };  // ±20 around base velocity, 10s period
LFO volLFO = { 0.5, 0.05, 40, 100 };  // CC7 volume for pad, 20s period

unsigned long lastLFO = 0;

// Update an LFO, computing its current output
uint8_t updateLFO(LFO &lfo, unsigned long dtMs) {
  // advance phase
  lfo.phase += lfo.freq * (dtMs / 1000.0);
  if (lfo.phase >= 1.0) lfo.phase -= 1.0;
  // sine from –1 to +1
  float s = sin(2 * PI * lfo.phase);
  // map to [minVal, maxVal]
  float mid  = (lfo.maxVal + lfo.minVal) / 2.0;
  float amp  = (lfo.maxVal - lfo.minVal) / 2.0;
  return constrain((int)(mid + amp * s), 0, 127);
}

#define MIDI_NOTE_ON    0x90
#define MIDI_NOTE_OFF   0x80
#define MIDI_CC         0xB0 

// Utility: send Control Change
void midiCC(uint8_t chan, uint8_t cc, uint8_t val) {
  if (chan > 15 || cc > 127 || val > 127) return;
  VS1053_MIDI.write(MIDI_CC | chan);
  VS1053_MIDI.write(cc);
  VS1053_MIDI.write(val);
}

const int ledPin = 5;  // the number of the LED pin

// Variables will change:
int ledState = LOW;  // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated

// constants won't change:
const long interval = 1000;  // interval at which to blink (milliseconds)

#define TRIG_PIN 3
#define ECHO_PIN 4

#define TRIG_PIN2 10 
#define ECHO_PIN2 9

long getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = (duration*.0343)/2;
  Serial.print("Distance Sensor 1: ");
  Serial.println(distance);
  return constrain(distance, 5, 100);    // limit range between 5 cm and 100 cm
}

long getDistanceSecondSensor() { 
  digitalWrite(TRIG_PIN2, LOW); 
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN2, LOW);

  long duration = pulseIn(ECHO_PIN2, HIGH);
  long distance = (duration*.0343)/2;
  //Serial.print("Distance Sensor 2: ");
  //Serial.println(distance);
  return constrain(distance, 5, 100); 
}

void setup() {
  Serial.begin(9600);
  Serial.println("VS1053 MIDI test");
  
  VS1053_MIDI.begin(31250); // MIDI uses a 'strange baud rate'
  
  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);
  
  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, VS1053_GM1_ECHOES);
  midiSetChannelVolume(0, 200);

  lastLFO = millis();

  pinMode(ledPin, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
pinMode(ECHO_PIN, INPUT);
}

// Pentatonic C major: C4, D4, E4, G4, A4
// const uint8_t scaleNotes[] = { 60, 62, 64, 67, 69 };
// const uint8_t S = sizeof(scaleNotes);

// // Markov transition matrix P[i][j] = P(next=j | current=i)
// const float P[S][S] = {
//   {0.2, 0.2, 0.3, 0.2, 0.1},
//   {0.2, 0.2, 0.3, 0.2, 0.1},
//   {0.1, 0.2, 0.4, 0.2, 0.1},
//   {0.1, 0.2, 0.2, 0.3, 0.2},
//   {0.1, 0.2, 0.2, 0.3, 0.2}

// };
// Dreamy whole tone scale across two octaves: C, D, E, F#, G#, A# (x2)
const uint8_t scaleNotes[] = { 
  36,  40, 43, 48,  50, 55, 60, 62, 67, 72, 74, 79 
};
const uint8_t S = sizeof(scaleNotes);

const float P[S][S] = {
  {0.05, 0.2, 0.2, 0.15, 0.15, 0.1, 0.05, 0.05, 0.05, 0.025, 0.025, 0.0},
  {0.1, 0.1, 0.2, 0.2, 0.15, 0.1, 0.05, 0.05, 0.05, 0.025, 0.025, 0.0},
  {0.1, 0.1, 0.2, 0.2, 0.15, 0.1, 0.05, 0.05, 0.05, 0.025, 0.025, 0.0},
  {0.05, 0.05, 0.1, 0.2, 0.2, 0.15, 0.1, 0.05, 0.05, 0.025, 0.025, 0.0},
  {0.05, 0.05, 0.05, 0.1, 0.2, 0.2, 0.15, 0.1, 0.05, 0.05, 0.025, 0.025},
  {0.05, 0.05, 0.05, 0.05, 0.1, 0.2, 0.2, 0.15, 0.1, 0.05, 0.05, 0.05},
  {0.025, 0.025, 0.05, 0.05, 0.05, 0.1, 0.2, 0.2, 0.15, 0.1, 0.05, 0.05},
  {0.0, 0.025, 0.025, 0.05, 0.05, 0.05, 0.1, 0.2, 0.2, 0.15, 0.1, 0.05},
  {0.0, 0.0, 0.025, 0.025, 0.05, 0.05, 0.1, 0.2, 0.2, 0.15, 0.1, 0.05},
  {0.0, 0.0, 0.0, 0.025, 0.025, 0.05, 0.05, 0.1, 0.2, 0.2, 0.15, 0.15},
  {0.0, 0.0, 0.0, 0.0, 0.025, 0.025, 0.05, 0.05, 0.1, 0.2, 0.2, 0.3},
  {0.0, 0.0, 0.0, 0.0, 0.0, 0.025, 0.025, 0.05, 0.05, 0.1, 0.2, 0.525}
};

// Sample next index from Markov row
uint8_t chooseNext(uint8_t current) {
  float r = random(0, 10001) / 10000.0;  // uniform [0,1]
  float cum = 0;
  for (uint8_t i = 0; i < S; i++) {
    cum += P[current][i];
    if (r <= cum) return i;
  }
  return S - 1;
}

// Melody state
uint8_t currentIdx    = 0;           // start on C4
unsigned long lastMel = 0;
const unsigned long MIN_DUR = 200;   // ms
const unsigned long MAX_DUR = 800;   // ms
const uint8_t BASE_VEL  = 70;        // centre velocity

// Drone (pad) state
uint8_t  padNote = 48;   // C2
bool     padOn   = false;
unsigned long padTime = 0;

// Play or stop a slow drone at irregular intervals
void pad() {
  unsigned long now = millis();
  if (!padOn && now - padTime > 5000 + random(10000)) {
    midiNoteOn (1, padNote, 40);
    padOn   = true;
    padTime = now;
  }
  else if ( padOn && now - padTime > 15000 + random(10000)) {
    midiNoteOff(1, padNote,  0);
    padOn   = false;
    padTime = now;
  }
}



void loop() {
  
  unsigned long now = millis();

  // --- update LFOs every cycle ---
  unsigned long dt = now - lastLFO;
  if (dt > 20) {  // 50 Hz update
    // pad volume LFO via CC7 on channel 1
    uint8_t vol = updateLFO(volLFO, dt);
    midiCC(1, 7, vol);

    // velocity LFO for melody
    uint8_t velOffset = updateLFO(velLFO, dt);
    // we'll apply this below

    lastLFO = now;
  }

  // maintain the drone
  pad();

  // melody generator
  // if (now - lastMel > MIN_DUR + random(MAX_DUR - MIN_DUR)) {
  //   currentIdx = chooseNext(currentIdx);
  //   uint8_t note = scaleNotes[currentIdx];

  //   // apply LFO offset around BASE_VEL
  //   int v = BASE_VEL + (int)updateLFO(velLFO, now - lastLFO);
  //   v = constrain(v, 10, 127);

  //   midiNoteOn (0, note, v);
  //   digitalWrite(ledPin, HIGH);
  //   delay(100 + random(200));
  //   midiNoteOff(0, note,  0);
  //   digitalWrite(ledPin, LOW);

  //   lastMel = now;
  // }
  long distance = getDistanceCM();

  long distance2 = getDistanceSecondSensor();

// Map distance to speed and loudness
// Closer = faster and louder
unsigned long melodyInterval = map(distance, 5, 100, 200, 1000);  // Shorter distance = faster notes
int dynamicVelocity = map(distance, 5, 100, 100, 40);             // Shorter = louder
dynamicVelocity = constrain(dynamicVelocity, 10, 127);

if (now - lastMel > melodyInterval) {
  currentIdx = chooseNext(currentIdx);
  uint8_t note = scaleNotes[currentIdx];

  if (distance2 < 50) {
    uint8_t harmonyNote = note + map(distance2, 5, 50, 12, 3); // Octave to minor third
    midiNoteOn(0, harmonyNote, dynamicVelocity);
}

  digitalWrite(ledPin, HIGH);
  delay(100 + random(100));
  midiNoteOff(0, note, 0);
  digitalWrite(ledPin, LOW);

  lastMel = now;
}

}

void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);  
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