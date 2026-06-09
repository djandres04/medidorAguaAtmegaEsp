#include <Wire.h>
#include "i2c_comm.h"

void i2cBegin() {
  Wire.begin();
}

void i2cSendDistance(uint8_t address, long distanceCm, bool valid, bool belowMin, bool aboveMax) {
  Wire.beginTransmission(address);
  Wire.write((uint8_t)0x01);
  Wire.write((uint8_t)(valid ? 1 : 0));
  Wire.write((uint8_t)belowMin);
  Wire.write((uint8_t)aboveMax);

  if (valid) {
    Wire.write((uint8_t)(distanceCm >> 8));
    Wire.write((uint8_t)(distanceCm & 0xFF));
  } else {
    Wire.write((uint8_t)0);
    Wire.write((uint8_t)0);
  }

  Wire.endTransmission();
}
