#include "i2c_master.h"
#include <Wire.h>

void i2cMasterBegin(uint8_t sdaPin, uint8_t sclPin, uint32_t frequency) {
  Wire.begin(sdaPin, sclPin, frequency);
}

bool readAtmegaStatus(uint8_t address, AtmegaStatus& status) {
  constexpr uint8_t EXPECTED_BYTES = 5;
  Wire.requestFrom((int)address, EXPECTED_BYTES);

  unsigned long start = millis();
  while (Wire.available() < EXPECTED_BYTES && millis() - start < 100) {
    delay(1);
  }

  if (Wire.available() < EXPECTED_BYTES) {
    return false;
  }

  status.valid = Wire.read() != 0;
  status.belowMin = Wire.read() != 0;
  status.aboveMax = Wire.read() != 0;
  uint16_t high = Wire.read();
  uint16_t low = Wire.read();
  status.distanceCm = (long)((high << 8) | low);

  return true;
}

String atmegaStatusToJson(const AtmegaStatus& status) {
  return String("{") +
         "\"valid\":" + (status.valid ? "true" : "false") + "," +
         "\"belowMin\":" + (status.belowMin ? "true" : "false") + "," +
         "\"aboveMax\":" + (status.aboveMax ? "true" : "false") + "," +
         "\"distanceCm\":" + status.distanceCm +
         "}";
}
