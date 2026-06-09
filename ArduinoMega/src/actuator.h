#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <Arduino.h>

void actuatorBegin(uint8_t pin);
void actuatorSetState(bool enabled);
bool actuatorUpdate(long distanceCm, long minDistance, long maxDistance);

#endif // ACTUATOR_H
