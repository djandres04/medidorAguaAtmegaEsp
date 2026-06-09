#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

// Inicializa la conexión WiFi con el SSID y contraseña proporcionados
// Nota: para WiFi abierta, pasar contraseña como cadena vacía ""
void setup_wifi(const char* ssid, const char* password);

// Verifica si el ESP32 está conectado al WiFi (útil para reconexiones)
bool is_wifi_connected();

// Configura una IP estática (debe llamarse ANTES de setup_wifi)
// Ejemplo: setup_static_ip("192.168.1.100", "192.168.1.1", "255.255.255.0", "8.8.8.8", "8.8.4.4");
// dns2 es opcional (pasar nullptr o cadena vacía si no se desea segundo DNS)
void setup_static_ip(const char* ip, const char* gateway, const char* subnet, const char* dns1, const char* dns2 = nullptr);

// Obtiene la dirección MAC del ESP32 (útil para registro en redes universitarias)
void print_mac_address();

#endif // WIFI_MANAGER_H