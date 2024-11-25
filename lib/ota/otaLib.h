#include <Arduino.h>

#ifndef OTA_LIB
#define OTA_LIB

void OTA_Init(const char* password, uint16_t port);
void arduino_OTA_Handle();

#endif