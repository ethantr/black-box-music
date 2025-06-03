#include "ultrasonic.h"

UltrasonicSensor::UltrasonicSensor(uint8_t trigPin, uint8_t echoPin, const char* name)
  : trig(trigPin), echo(echoPin), sensorName(name) {}

void UltrasonicSensor::begin() {
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
}

long UltrasonicSensor::getDistanceCM() {
  const int MIN_DISTANCE_CM = 1;
const int MAX_DISTANCE_CM = 400;
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH);         // Time in microseconds
  float speedOfSound = 0.0343;                 // cm per microsecond
  float distanceCM = (duration * speedOfSound) / 2.0; // One-way trip
  

/*
  if (DEBUG_ULTRASONIC) {
    Serial.print("[");
  Serial.print(sensorName);
  Serial.print("] Distance: ");
  Serial.print(distanceCM);
  Serial.println(" cm");
}*/

  return constrain(distanceCM, MIN_DISTANCE_CM, MAX_DISTANCE_CM);
}