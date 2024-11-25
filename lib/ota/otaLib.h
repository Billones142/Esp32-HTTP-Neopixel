#pragma once
#include <Arduino.h>


void OTA_Init(const char* password, uint16_t port);
void arduino_OTA_Handle();