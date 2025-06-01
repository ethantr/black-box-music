#pragma once
#include <Arduino.h>

#define DEBUG_ULTRASONIC true

class UltrasonicSensor {
public:
  UltrasonicSensor(uint8_t trigPin, uint8_t echoPin, const char* name = "");
  void begin();
  long getDistanceCM();

private:
  uint8_t trig, echo;
  const char* sensorName;
};

