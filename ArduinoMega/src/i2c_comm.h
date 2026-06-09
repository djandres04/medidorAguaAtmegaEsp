#ifndef I2C_COMM_H
#define I2C_COMM_H

#include <Arduino.h>

void i2cBegin();
void i2cSendDistance(uint8_t address, long distanceCm, bool valid, bool belowMin, bool aboveMax);

#endif // I2C_COMM_H
