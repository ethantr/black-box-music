#include "PressureSensor.h"

PressureSensor::PressureSensor(uint8_t input, const char* name)
  : inputPin(input), sensorName(name) {}

void PressureSensor::begin(){
  pinMode(inputPin, INPUT);
}
long PressureSensor::read() {
  long pressure = analogRead(inputPin);
  
  if (DEBUG_SENSOR) {
    Serial.print("[");
  Serial.print(sensorName);
  Serial.print("] ADC: ");
  Serial.println(pressure);
  }
  return pressure;
}