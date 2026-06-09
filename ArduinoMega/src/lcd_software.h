#ifndef LCD_SOFTWARE_H
#define LCD_SOFTWARE_H

#include <Arduino.h>

// Minimal software-I2C LCD driver (PCF8574 backpack) using SoftwareWire
// Designed to run on ATmega while keeping hardware TWI free for ESP I2C.

void lcdSoftBegin(uint8_t address, uint8_t cols, uint8_t rows, uint8_t sdaPin = 10, uint8_t sclPin = 11, bool backlight = true);
void lcdSoftClear();
void lcdSoftPrintLine(uint8_t line, const char* text);
void lcdSoftShowStatus(long distanceCm, bool valid, bool belowMin, bool aboveMax);

#endif // LCD_SOFTWARE_H
