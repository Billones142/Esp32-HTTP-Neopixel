#pragma once
#include <config.h>

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <luaNeopixel.h>
extern Adafruit_NeoPixel pixels;

void neopixel_Init(uint16_t pixelAmount, uint16_t pin);

void processJsonToNeopixelStatic(Adafruit_NeoPixel &pixels, String jsonString);
void processJsonToNeopixelScript(Adafruit_NeoPixel &pixels, String jsonString);

JsonDocument getSavedLuaScripts(bool retrieveScriptContent = false);
JsonDocument getSavedLuaScripts(fs::FS &fs, bool retrieveScriptContent = false);