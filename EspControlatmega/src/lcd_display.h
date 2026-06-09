#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <Arduino.h>
#include "i2c_master.h"

void lcdBegin(uint8_t address, uint8_t cols, uint8_t rows);
void lcdShowStatus(const AtmegaStatus& status);
void lcdShowMessage(const char* line1, const char* line2);

#endif // LCD_DISPLAY_H
