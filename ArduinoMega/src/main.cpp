#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include "ultrasonic.h"
#include "i2c_comm.h"
#include "actuator.h"
#include "lcd_software.h"

// Pines del sensor JSN-SR04T
static const uint8_t TRIG_PIN = 22;
static const uint8_t ECHO_PIN = 23;

// Pin del actuador (bomba, válvula, relé, etc.)
static const uint8_t ACTUATOR_PIN = 24;

// Dirección I2C del ATmega como esclavo
static const uint8_t I2C_ADDRESS = 0x08;

// Valores ajustables en tiempo de ejecución
static long minDistanceCm = 10;
static long maxDistanceCm = 80;
// Intervalo de medición en milisegundos (medición local del ATmega cada 1 minuto)
static unsigned long measurementIntervalMs = 60UL * 1000UL; // 60000 ms = 1 min

// EEPROM layout and helpers
static const uint8_t EEPROM_MAGIC_ADDR = 0;
static const uint8_t EEPROM_MIN_HI = 1;
static const uint8_t EEPROM_MIN_LO = 2;
static const uint8_t EEPROM_MAX_HI = 3;
static const uint8_t EEPROM_MAX_LO = 4;
static const uint8_t EEPROM_INT_HI = 5;
static const uint8_t EEPROM_INT_LO = 6;
static const uint8_t EEPROM_MAGIC = 0xA5;

void loadSettingsFromEEPROM() {
  if (EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC) {
    uint16_t min_h = EEPROM.read(EEPROM_MIN_HI);
    uint16_t min_l = EEPROM.read(EEPROM_MIN_LO);
    minDistanceCm = (long)((min_h << 8) | min_l);

    uint16_t max_h = EEPROM.read(EEPROM_MAX_HI);
    uint16_t max_l = EEPROM.read(EEPROM_MAX_LO);
    maxDistanceCm = (long)((max_h << 8) | max_l);

    uint16_t int_h = EEPROM.read(EEPROM_INT_HI);
    uint16_t int_l = EEPROM.read(EEPROM_INT_LO);
    uint16_t intervalSec = (uint16_t)((int_h << 8) | int_l);
    if (intervalSec > 0) measurementIntervalMs = (unsigned long)intervalSec * 1000UL;

    Serial.println("EEPROM: settings loaded");
  } else {
    Serial.println("EEPROM: no valid settings, using defaults");
  }
}

void saveSettingsToEEPROM() {
  EEPROM.update(EEPROM_MAGIC_ADDR, EEPROM_MAGIC);
  EEPROM.update(EEPROM_MIN_HI, (uint8_t)(minDistanceCm >> 8));
  EEPROM.update(EEPROM_MIN_LO, (uint8_t)(minDistanceCm & 0xFF));
  EEPROM.update(EEPROM_MAX_HI, (uint8_t)(maxDistanceCm >> 8));
  EEPROM.update(EEPROM_MAX_LO, (uint8_t)(maxDistanceCm & 0xFF));
  uint16_t intervalSec = (uint16_t)(measurementIntervalMs / 1000UL);
  EEPROM.update(EEPROM_INT_HI, (uint8_t)(intervalSec >> 8));
  EEPROM.update(EEPROM_INT_LO, (uint8_t)(intervalSec & 0xFF));
  Serial.println("EEPROM: settings saved");
}

// LCD (Software I2C) settings
static const uint8_t LCD_I2C_ADDRESS = 0x27;
static const uint8_t LCD_SOFT_SDA_PIN = 10;
static const uint8_t LCD_SOFT_SCL_PIN = 11;

static UltrasonicReading lastReading = { -1, false, false, false };
static unsigned long lastMeasurementMs = 0;

// Snapshot variables (read by I2C request ISR) - kept small and atomic
volatile uint8_t snap_valid = 0;
volatile uint8_t snap_belowMin = 0;
volatile uint8_t snap_aboveMax = 0;
volatile uint16_t snap_distanceCm = 0;
volatile uint16_t snap_minDistance = 0;
volatile uint16_t snap_maxDistance = 0;
volatile uint16_t snap_intervalSec = 0;

// Trigger flag: ESP can request a forced measurement (deferred, non-blocking)
volatile bool flagTriggerMeasure = false;

uint16_t readUint16FromWire() {
  uint16_t high = Wire.read();
  uint16_t low = Wire.read();
  return (high << 8) | low;
}

// Use volatile flags to avoid heavy operations in ISR context
volatile bool flagMinUpdated = false;
volatile bool flagMaxUpdated = false;
volatile bool flagIntervalUpdated = false;
volatile uint16_t newMinValue = 0;
volatile uint16_t newMaxValue = 0;
volatile uint16_t newIntervalSec = 0;

void handleI2CReceive(int count) {
  if (count < 1) return;

  uint8_t command = Wire.read();

  switch (command) {
    case I2C_CMD_SET_MIN:
      if (count >= 3 && Wire.available() >= 2) {
        newMinValue = readUint16FromWire();
        flagMinUpdated = true;
      }
      break;

    case I2C_CMD_SET_MAX:
      if (count >= 3 && Wire.available() >= 2) {
        newMaxValue = readUint16FromWire();
        flagMaxUpdated = true;
      }
      break;

    case I2C_CMD_SET_INTERVAL:
      if (count >= 3 && Wire.available() >= 2) {
        newIntervalSec = readUint16FromWire();
        flagIntervalUpdated = true;
      }
      break;

    case I2C_CMD_TRIGGER_MEASURE:
      // Request a forced measurement — handled in main loop (deferred)
      flagTriggerMeasure = true;
      break;

    case I2C_CMD_REQUEST_STATUS:
      // maestro preguntará vía request; no hacer nada aquí
      break;

    default:
      // Ignorar comando desconocido en ISR
      break;
  }
}

void handleI2CRequest() {
  // Read snapshot variables and send them (fast, ISR-safe)
  i2cSendStatus((bool)snap_valid,
                (bool)snap_belowMin,
                (bool)snap_aboveMax,
                (long)snap_distanceCm,
                (long)snap_minDistance,
                (long)snap_maxDistance,
                (unsigned long)snap_intervalSec * 1000UL);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Serial.println("Inicializando medidor de agua en modo esclavo I2C...");

  ultrasonicBegin(TRIG_PIN, ECHO_PIN);
  actuatorBegin(ACTUATOR_PIN);
  i2cBegin(I2C_ADDRESS);
  i2cSetHandlers(handleI2CReceive, handleI2CRequest);

  // Cargar ajustes persistentes
  loadSettingsFromEEPROM();

  // Inicializar LCD mediante Software I2C en pines designados
  lcdSoftBegin(LCD_I2C_ADDRESS, 16, 2, LCD_SOFT_SDA_PIN, LCD_SOFT_SCL_PIN, true);
  lcdSoftPrintLine(0, "Medidor de agua");
  char buf[17];
  snprintf(buf, sizeof(buf), "Min:%4ld Max:%4ld", minDistanceCm, maxDistanceCm);
  lcdSoftPrintLine(1, buf);
}

void loop() {
  // If ESP requested an immediate measurement, do it now (deferred, non-blocking in ISR)
  bool doForced = false;
  noInterrupts();
  if (flagTriggerMeasure) {
    doForced = true;
    flagTriggerMeasure = false;
  }
  interrupts();

  if (doForced) {
    lastMeasurementMs = millis();
    lastReading = ultrasonicMeasure(TRIG_PIN, ECHO_PIN, minDistanceCm, maxDistanceCm);
  } else {
    if (millis() - lastMeasurementMs < measurementIntervalMs) {
      return;
    }
    lastMeasurementMs = millis();
    lastReading = ultrasonicMeasure(TRIG_PIN, ECHO_PIN, minDistanceCm, maxDistanceCm);
  }

  if (lastReading.valid) {
    Serial.print("Distancia: ");
    Serial.print(lastReading.distanceCm);
    Serial.println(" cm");

    if (lastReading.belowMin) {
      Serial.println("Estado: tanque lleno o nivel demasiado alto.");
    } else if (lastReading.aboveMax) {
      Serial.println("Estado: tanque bajo, se debe llenar.");
    } else {
      Serial.println("Estado: nivel dentro del rango aceptable.");
    }
  } else {
    Serial.println("Lectura ultrasonica invalida.");
  }

  bool actuatorOn = actuatorUpdate(lastReading.distanceCm, minDistanceCm, maxDistanceCm);
  Serial.print("Actuador: ");
  Serial.println(actuatorOn ? "ENCENDIDO" : "APAGADO");

  // Si hubo cambios por I2C, aplicarlos en el contexto principal y mostrar en LCD
  if (flagMinUpdated) {
    minDistanceCm = (long)newMinValue;
    flagMinUpdated = false;
    saveSettingsToEEPROM();
    char buf[17];
    snprintf(buf, sizeof(buf), "Min:%4ldcm", minDistanceCm);
    lcdSoftPrintLine(0, buf);
  }
  if (flagMaxUpdated) {
    maxDistanceCm = (long)newMaxValue;
    flagMaxUpdated = false;
    saveSettingsToEEPROM();
    char buf[17];
    snprintf(buf, sizeof(buf), "Max:%4ldcm", maxDistanceCm);
    lcdSoftPrintLine(1, buf);
  }
  if (flagIntervalUpdated) {
    measurementIntervalMs = (unsigned long)newIntervalSec * 1000UL;
    flagIntervalUpdated = false;
    // show short message
    char buf[17];
    snprintf(buf, sizeof(buf), "Interval:%us", newIntervalSec);
    lcdSoftPrintLine(1, buf);
    saveSettingsToEEPROM();
  }

  // Actualizar LCD con el estado actual
  // Update snapshot atomically for ISR to read
  noInterrupts();
  snap_valid = lastReading.valid ? 1 : 0;
  snap_belowMin = lastReading.belowMin ? 1 : 0;
  snap_aboveMax = lastReading.aboveMax ? 1 : 0;
  snap_distanceCm = (uint16_t)lastReading.distanceCm;
  snap_minDistance = (uint16_t)minDistanceCm;
  snap_maxDistance = (uint16_t)maxDistanceCm;
  snap_intervalSec = (uint16_t)(measurementIntervalMs / 1000UL);
  interrupts();

  // Now update the LCD (Software I2C) with human-friendly information
  lcdSoftShowStatus(lastReading.distanceCm, lastReading.valid, lastReading.belowMin, lastReading.aboveMax);
}
