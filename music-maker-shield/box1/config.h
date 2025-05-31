#pragma once


// Pin definitions
#define VS1053_RX      2 // connects to VS1053 RX
#define VS1053_RESET   9 // connects to VS1053 RESET (if needed)
#define TRIG_PIN       3
#define ECHO_PIN       4
#define LED_PIN        5
#define PRESSURE_PIN   A3

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

const uint8_t BASE_VEL = 70;
unsigned long lastLFO = 0;
unsigned long lastMel = 0;
