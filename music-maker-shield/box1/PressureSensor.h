#pragma once
#include <Arduino.h>

#define DEBUG_SENSOR false

class PressureSensor {
public:
  PressureSensor(uint8_t input, const char* name = "");
  void begin();
  long read();

private:
  uint8_t inputPin;
  const char* sensorName;
};

