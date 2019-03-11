#ifndef WEBSOCKTOOLS_H_
#define WEBSOCKTOOLS_H_

#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "SmuffIfcConfig.h"
#include "Cipher.h"
#include "mbedtls/aes.h"
#include "mbedtls/base64.h"

#ifdef __cplusplus
extern "C" {
#endif

int  scanNetworks();
void parseSetting(const char* data, AsyncWebSocketClient* client);
void parseCommand(const char* data, AsyncWebSocketClient* client);

#ifdef __cplusplus
}
#endif

#endif