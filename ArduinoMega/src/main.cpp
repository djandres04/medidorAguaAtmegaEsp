#include <Arduino.h>
#include "ultrasonic.h"
#include "i2c_comm.h"
#include "actuator.h"

// Pines del sensor JSN-SR04T
static const uint8_t TRIG_PIN = 22;
static const uint8_t ECHO_PIN = 23;

// Pin del actuador (bomba, válvula, relé, etc.)
static const uint8_t ACTUATOR_PIN = 24;

// Dirección I2C del dispositivo receptor
static const uint8_t I2C_ADDRESS = 0x08;

// Rango mínimo y máximo en centímetros
// MIN_DISTANCE_CM = nivel más alto permitido
// MAX_DISTANCE_CM = nivel más bajo permitido antes de activar la bomba
static const long MIN_DISTANCE_CM = 10;
static const long MAX_DISTANCE_CM = 80;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  }

  Serial.println("Inicializando medidor de agua...");

  ultrasonicBegin(TRIG_PIN, ECHO_PIN);
  actuatorBegin(ACTUATOR_PIN);
  i2cBegin();
}

void loop() {
  UltrasonicReading reading = ultrasonicMeasure(TRIG_PIN, ECHO_PIN, MIN_DISTANCE_CM, MAX_DISTANCE_CM);

  if (reading.valid) {
    Serial.print("Distancia: ");
    Serial.print(reading.distanceCm);
    Serial.println(" cm");

    if (reading.belowMin) {
      Serial.println("Estado: tanque lleno o nivel demasiado alto.");
    } else if (reading.aboveMax) {
      Serial.println("Estado: tanque bajo, se debe llenar.");
    } else {
      Serial.println("Estado: nivel dentro del rango aceptable.");
    }
  } else {
    Serial.println("Lectura ultrasonica invalida.");
  }

  bool actuatorOn = actuatorUpdate(reading.distanceCm, MIN_DISTANCE_CM, MAX_DISTANCE_CM);
  Serial.print("Actuador: ");
  Serial.println(actuatorOn ? "ENCENDIDO" : "APAGADO");

  i2cSendDistance(I2C_ADDRESS, reading.distanceCm, reading.valid, reading.belowMin, reading.aboveMax);

  delay(500);
}
