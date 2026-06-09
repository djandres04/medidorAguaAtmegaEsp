#include "wifi_manager.h"
#include <WiFi.h>
#include <string.h>

void setup_wifi(const char* ssid, const char* password) {
    Serial.print("Conectando a la red WiFi: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA); // Configura el ESP32 como estación (cliente WiFi)
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi conectado exitosamente.");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
}

bool is_wifi_connected() {
    return WiFi.status() == WL_CONNECTED;
}

void setup_static_ip(const char* ip, const char* gateway, const char* subnet, const char* dns1, const char* dns2) {
    IPAddress staticIP;
    IPAddress gatewayIP;
    IPAddress subnetMask;
    IPAddress dns1IP;
    IPAddress dns2IP;
    
    staticIP.fromString(ip);
    gatewayIP.fromString(gateway);
    subnetMask.fromString(subnet);
    dns1IP.fromString(dns1);
    if (dns2 != nullptr && strlen(dns2) > 0) dns2IP.fromString(dns2);
    
    if (dns2 != nullptr && strlen(dns2) > 0) {
        WiFi.config(staticIP, gatewayIP, subnetMask, dns1IP, dns2IP);
    } else {
        WiFi.config(staticIP, gatewayIP, subnetMask, dns1IP);
    }
    Serial.print("IP estática configurada: ");
    Serial.println(ip);
}

void print_mac_address() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    Serial.print("MAC Address: ");
    for (int i = 0; i < 6; i++) {
        if (mac[i] < 16) Serial.print("0");
        Serial.print(mac[i], HEX);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
}