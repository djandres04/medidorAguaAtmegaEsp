#include "i2c_master.h"
#include <Wire.h>

void i2cMasterBegin(uint8_t sdaPin, uint8_t sclPin, uint32_t frequency) {
  Wire.begin(sdaPin, sclPin, frequency);
}

bool readAtmegaStatus(uint8_t address, AtmegaStatus& status) {
  // Now the slave sends a 12-byte packet:
  // [cmd][valid][below][above][dist_h][dist_l][min_h][min_l][max_h][max_l][int_h][int_l]
  constexpr uint8_t EXPECTED_BYTES = 12;
  Wire.requestFrom((uint8_t)address, (uint8_t)EXPECTED_BYTES);

  unsigned long start = millis();
  while (Wire.available() < EXPECTED_BYTES && millis() - start < 100) {
    delay(1);
  }

  if (Wire.available() < EXPECTED_BYTES) {
    return false;
  }

  // First byte is response command, skip it
  uint8_t respCmd = Wire.read(); (void)respCmd;

  status.valid = Wire.read() != 0;
  status.belowMin = Wire.read() != 0;
  status.aboveMax = Wire.read() != 0;

  uint16_t high = Wire.read();
  uint16_t low = Wire.read();
  status.distanceCm = (long)((high << 8) | low);

  uint16_t min_h = Wire.read();
  uint16_t min_l = Wire.read();
  status.minDistanceCm = (uint16_t)((min_h << 8) | min_l);

  uint16_t max_h = Wire.read();
  uint16_t max_l = Wire.read();
  status.maxDistanceCm = (uint16_t)((max_h << 8) | max_l);

  uint16_t int_h = Wire.read();
  uint16_t int_l = Wire.read();
  status.intervalSec = (uint16_t)((int_h << 8) | int_l);

  return true;
}

void sendTriggerMeasure(uint8_t address) {
  Wire.beginTransmission((uint8_t)address);
  Wire.write((uint8_t)0x30); // I2C_CMD_TRIGGER_MEASURE
  Wire.endTransmission();
}

String atmegaStatusToJson(const AtmegaStatus& status) {
  return String("{") +
         "\"valid\":" + (status.valid ? "true" : "false") + "," +
         "\"belowMin\":" + (status.belowMin ? "true" : "false") + "," +
         "\"aboveMax\":" + (status.aboveMax ? "true" : "false") + "," +
         "\"distanceCm\":" + status.distanceCm + "," +
         "\"minDistanceCm\":" + status.minDistanceCm + "," +
         "\"maxDistanceCm\":" + status.maxDistanceCm + "," +
         "\"intervalSec\":" + status.intervalSec +
         "}";
}
