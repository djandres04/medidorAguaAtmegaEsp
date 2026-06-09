#include "lcd_display.h"
#include <LiquidCrystal_I2C.h>

static LiquidCrystal_I2C* lcd = nullptr;

void lcdBegin(uint8_t address, uint8_t cols, uint8_t rows) {
  if (lcd != nullptr) {
    delete lcd;
  }
  lcd = new LiquidCrystal_I2C(address, cols, rows);
  lcd->init();
  lcd->backlight();
  lcd->clear();
}

void lcdShowStatus(const AtmegaStatus& status) {
  if (lcd == nullptr) {
    return;
  }

  char line1[17];
  char line2[17];

  snprintf(line1, sizeof(line1), "Dist: %ld cm", status.distanceCm);

  if (!status.valid) {
    strncpy(line2, "Sensor invalido  ", sizeof(line2));
  } else if (status.aboveMax) {
    strncpy(line2, "Nivel bajo      ", sizeof(line2));
  } else if (status.belowMin) {
    strncpy(line2, "Tanque lleno    ", sizeof(line2));
  } else {
    strncpy(line2, "Nivel ok        ", sizeof(line2));
  }

  line2[16] = '\0';

  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(line1);
  lcd->setCursor(0, 1);
  lcd->print(line2);
}

void lcdShowMessage(const char* line1, const char* line2) {
  if (lcd == nullptr) {
    return;
  }
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(line1);
  lcd->setCursor(0, 1);
  lcd->print(line2);
}
