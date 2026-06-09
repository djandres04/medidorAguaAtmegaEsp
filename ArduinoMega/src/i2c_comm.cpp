#include <Wire.h>
#include "i2c_comm.h"

static void (*receiveHandlerFn)(int) = nullptr;
static void (*requestHandlerFn)() = nullptr;

void i2cBegin(uint8_t address) {
  Wire.begin(address);
}

void i2cSetHandlers(void (*receiveHandler)(int), void (*requestHandler)()) {
  receiveHandlerFn = receiveHandler;
  requestHandlerFn = requestHandler;
  Wire.onReceive(receiveHandlerFn);
  Wire.onRequest(requestHandlerFn);
}

void i2cSendStatus(bool valid, bool belowMin, bool aboveMax, long distanceCm, long minDistanceCm, long maxDistanceCm, unsigned long intervalMs) {
  Wire.write((uint8_t)I2C_CMD_RESPONSE_STATUS);
  Wire.write((uint8_t)(valid ? 1 : 0));
  Wire.write((uint8_t)(belowMin ? 1 : 0));
  Wire.write((uint8_t)(aboveMax ? 1 : 0));

  Wire.write((uint8_t)(distanceCm >> 8));
  Wire.write((uint8_t)(distanceCm & 0xFF));

  Wire.write((uint8_t)(minDistanceCm >> 8));
  Wire.write((uint8_t)(minDistanceCm & 0xFF));

  Wire.write((uint8_t)(maxDistanceCm >> 8));
  Wire.write((uint8_t)(maxDistanceCm & 0xFF));

  uint16_t intervalSec = intervalMs / 1000;
  Wire.write((uint8_t)(intervalSec >> 8));
  Wire.write((uint8_t)(intervalSec & 0xFF));
}
