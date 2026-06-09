#include "lcd_software.h"
#include <SoftwareWire.h>

// PCF8574 pin mapping compatible with LiquidCrystal_I2C used elsewhere:
// P7 P6 P5 P4 P3 P2 P1 P0
// D7 D6 D5 D4 BL EN RW RS

#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04  // Enable bit
#define Rw 0x02  // Read/Write bit (not used)
#define Rs 0x01  // Register select bit

static SoftwareWire* sWire = nullptr;
static uint8_t _Addr = 0;
static uint8_t _cols = 16;
static uint8_t _rows = 2;
static uint8_t _backlightval = LCD_BACKLIGHT;

static void expanderWrite(uint8_t data) {
  if (!sWire) return;
  sWire->beginTransmission(_Addr);
  sWire->write((int)(data) | _backlightval);
  sWire->endTransmission();
}

static void pulseEnable(uint8_t data) {
  expanderWrite(data | En);
  delayMicroseconds(1);
  expanderWrite(data & ~En);
  delayMicroseconds(50);
}

static void write4bits(uint8_t value) {
  expanderWrite(value);
  pulseEnable(value);
}

static void send(uint8_t value, uint8_t mode) {
  uint8_t highnib = value & 0xF0;
  uint8_t lownib = (value << 4) & 0xF0;
  write4bits(highnib | mode);
  write4bits(lownib | mode);
}

static void command(uint8_t value) {
  send(value, 0);
}

static void writeChar(uint8_t value) {
  send(value, Rs);
}

void lcdSoftBegin(uint8_t address, uint8_t cols, uint8_t rows, uint8_t sdaPin, uint8_t sclPin, bool backlight) {
  if (sWire) {
    delete sWire;
    sWire = nullptr;
  }
  sWire = new SoftwareWire(sdaPin, sclPin);
  sWire->begin();

  _Addr = address;
  _cols = cols;
  _rows = rows;
  _backlightval = backlight ? LCD_BACKLIGHT : LCD_NOBACKLIGHT;

  // Init sequence (adapted from LiquidCrystal_I2C)
  delay(50);
  expanderWrite(_backlightval);
  delay(1000);

  // put the LCD into 4 bit mode
  write4bits(0x03 << 4);
  delayMicroseconds(4500);
  write4bits(0x03 << 4);
  delayMicroseconds(4500);
  write4bits(0x03 << 4);
  delayMicroseconds(150);
  write4bits(0x02 << 4);

  // Function set
  command(0x20 | 0x08); // 4-bit, 2 lines
  // Display on
  command(0x08 | 0x04);
  // Clear
  command(0x01);
  delayMicroseconds(2000);
}

void lcdSoftClear() {
  command(0x01);
  delayMicroseconds(2000);
}

void lcdSoftPrintLine(uint8_t line, const char* text) {
  if (!sWire) return;
  if (line >= _rows) return;
  uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  command(0x80 | (row_offsets[line]));
  delayMicroseconds(50);

  // Print up to cols characters
  for (uint8_t i = 0; i < _cols; i++) {
    char c = text[i];
    if (c == '\0') break;
    writeChar(c);
  }
}

void lcdSoftShowStatus(long distanceCm, bool valid, bool belowMin, bool aboveMax) {
  char line1[17];
  char line2[17];
  if (!valid) {
    snprintf(line1, sizeof(line1), "Dist: -- cm");
    snprintf(line2, sizeof(line2), "Sensor invalido   ");
  } else {
    snprintf(line1, sizeof(line1), "Dist: %4ld cm", distanceCm);
    if (aboveMax) {
      snprintf(line2, sizeof(line2), "Nivel bajo        ");
    } else if (belowMin) {
      snprintf(line2, sizeof(line2), "Tanque lleno      ");
    } else {
      snprintf(line2, sizeof(line2), "Nivel ok          ");
    }
  }
  lcdSoftPrintLine(0, line1);
  lcdSoftPrintLine(1, line2);
}
