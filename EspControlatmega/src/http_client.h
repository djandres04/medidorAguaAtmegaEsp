#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>

// Función para enviar el JSON mediante HTTP POST a la URL indicada
void send_json_to_server(const char* url, const String& json_payload);

#endif // HTTP_CLIENT_H