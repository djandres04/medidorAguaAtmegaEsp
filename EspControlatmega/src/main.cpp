#include <Arduino.h>
#include <Wire.h>
#include "wifi_manager.h"
#include "http_client.h"
#include "i2c_master.h"

// --- Configuración de WiFi ---
const char* WIFI_SSID = "xxxxxx";
const char* WIFI_PASSWORD = "";

// --- Configuración del servidor ---
const char* API_URL = "http://aeronic.herokuapp.com/api/telemetry/";

// --- Configuración I2C ---
const uint8_t ATMEGA_I2C_ADDRESS = 0x08;
const uint8_t I2C_SDA_PIN = 21;
const uint8_t I2C_SCL_PIN = 22;

// --- Configuración de hardware ---
const int LED_PIN = 2;

// Intervalo de medición en milisegundos
const unsigned long MEASUREMENT_INTERVAL_MS = 60000;
unsigned long lastMeasurementMs = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("Iniciando ESP32 para leer ATmega por I2C...");

  i2cMasterBegin(I2C_SDA_PIN, I2C_SCL_PIN);
  setup_wifi(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Listo. Esperando la primera medicion...");
}

void loop() {
  if (millis() - lastMeasurementMs < MEASUREMENT_INTERVAL_MS) {
    delay(100);
    return;
  }
  lastMeasurementMs = millis();

  digitalWrite(LED_PIN, HIGH);

  if (!is_wifi_connected()) {
    Serial.println("WiFi desconectado. Reintentando conexion...");
    setup_wifi(WIFI_SSID, WIFI_PASSWORD);
  }

  AtmegaStatus status;
  bool ok = readAtmegaStatus(ATMEGA_I2C_ADDRESS, status);

  if (!ok) {
    Serial.println("Error I2C: no se pudieron leer los datos del ATmega.");
    digitalWrite(LED_PIN, LOW);
    return;
  }

  Serial.println("Datos recibidos del ATmega:");
  Serial.printf("  valid=%d belowMin=%d aboveMax=%d distance=%ld cm\n",
                status.valid, status.belowMin, status.aboveMax, status.distanceCm);

  String jsonPayload = atmegaStatusToJson(status);
  Serial.print("Payload JSON: ");
  Serial.println(jsonPayload);

  send_json_to_server(API_URL, jsonPayload);

  digitalWrite(LED_PIN, LOW);
}
