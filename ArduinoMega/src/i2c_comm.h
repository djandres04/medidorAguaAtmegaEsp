#ifndef I2C_COMM_H
#define I2C_COMM_H

#include <Arduino.h>

enum I2CCommand : uint8_t {
  I2C_CMD_REQUEST_STATUS = 0x10,
  I2C_CMD_RESPONSE_STATUS = 0x11,
  I2C_CMD_SET_MIN = 0x20,
  I2C_CMD_SET_MAX = 0x21,
  I2C_CMD_SET_INTERVAL = 0x22
  , I2C_CMD_TRIGGER_MEASURE = 0x30
};

void i2cBegin(uint8_t address);
void i2cSetHandlers(void (*receiveHandler)(int), void (*requestHandler)());
void i2cSendStatus(bool valid, bool belowMin, bool aboveMax, long distanceCm, long minDistanceCm, long maxDistanceCm, unsigned long intervalMs);

#endif // I2C_COMM_H
