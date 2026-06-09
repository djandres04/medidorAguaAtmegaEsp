#include "actuator.h"

static uint8_t actuatorPin = 0;
static bool actuatorState = false;

void actuatorBegin(uint8_t pin) {
  actuatorPin = pin;
  pinMode(actuatorPin, OUTPUT);
  actuatorState = false;
  digitalWrite(actuatorPin, LOW);
}

void actuatorSetState(bool enabled) {
  actuatorState = enabled;
  digitalWrite(actuatorPin, actuatorState ? HIGH : LOW);
}

bool actuatorUpdate(long distanceCm, long minDistance, long maxDistance) {
  bool shouldActivate = false;

  if (distanceCm < 0) {
    // Lectura inválida: no activar la bomba
    shouldActivate = false;
  } else if (distanceCm > maxDistance) {
    // Nivel bajo: activar la bomba para llenar el tanque
    shouldActivate = true;
  } else {
    // Nivel seguro o tanque lleno: detener la bomba
    shouldActivate = false;
  }

  actuatorSetState(shouldActivate);
  return actuatorState;
}
