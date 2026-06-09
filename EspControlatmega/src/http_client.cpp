#include "http_client.h"
#include <HTTPClient.h>
#include <WiFi.h>

void send_json_to_server(const char* url, const String& json_payload) {
    // Primero verificamos que seguimos conectados al WiFi
    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        
        http.begin(url); // Iniciamos la conexión con la URL
        http.addHeader("Content-Type", "application/json"); // Indicamos que enviamos un JSON

        // Hacemos la petición POST
        int httpResponseCode = http.POST(json_payload);

        if (httpResponseCode > 0) {
            Serial.printf("HTTP Response code: %d\n", httpResponseCode);
        } else {
            Serial.printf("Error en la petición HTTP: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end(); // Liberamos los recursos
    } else {
        Serial.println("Error: WiFi desconectado. No se pudo enviar el JSON.");
    }
}