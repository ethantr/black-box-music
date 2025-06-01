#pragma once
#include <Arduino.h>


struct Note {
  uint8_t pitch;
  uint8_t velocity;
  uint8_t channel;

  Note(uint8_t p, uint8_t v = 90, uint8_t c  = 0)
    : pitch(p), velocity(v), channel(c) {}
  Note() : pitch(0), velocity(0), channel(0) {} 
};