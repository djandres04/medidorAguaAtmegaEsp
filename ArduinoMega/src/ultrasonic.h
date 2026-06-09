#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <Arduino.h>

struct UltrasonicReading {
  long distanceCm;
  bool valid;
  bool belowMin;
  bool aboveMax;
};

void ultrasonicBegin(uint8_t trigPin, uint8_t echoPin);
UltrasonicReading ultrasonicMeasure(uint8_t trigPin, uint8_t echoPin, long minDistance, long maxDistance);
long ultrasonicReadDistance(uint8_t trigPin, uint8_t echoPin);

#endif // ULTRASONIC_H
