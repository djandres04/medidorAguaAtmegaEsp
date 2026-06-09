#ifndef I2C_MASTER_H
#define I2C_MASTER_H

#include <Arduino.h>

struct AtmegaStatus {
  bool valid;
  bool belowMin;
  bool aboveMax;
  long distanceCm;
};

void i2cMasterBegin(uint8_t sdaPin, uint8_t sclPin, uint32_t frequency = 100000);
bool readAtmegaStatus(uint8_t address, AtmegaStatus& status);
String atmegaStatusToJson(const AtmegaStatus& status);

#endif // I2C_MASTER_H
