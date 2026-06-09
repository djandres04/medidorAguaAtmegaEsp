#include "ultrasonic.h"

void ultrasonicBegin(uint8_t trigPin, uint8_t echoPin) {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
}

long ultrasonicReadDistance(uint8_t trigPin, uint8_t echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);
  if (duration <= 0) {
    return -1;
  }

  long distanceCm = duration * 0.0343 / 2;
  return distanceCm;
}

UltrasonicReading ultrasonicMeasure(uint8_t trigPin, uint8_t echoPin, long minDistance, long maxDistance) {
  UltrasonicReading reading;
  reading.distanceCm = ultrasonicReadDistance(trigPin, echoPin);
  reading.valid = (reading.distanceCm >= 0);
  reading.belowMin = false;
  reading.aboveMax = false;

  if (reading.valid) {
    reading.belowMin = (reading.distanceCm < minDistance);
    reading.aboveMax = (reading.distanceCm > maxDistance);
  }

  return reading;
}
