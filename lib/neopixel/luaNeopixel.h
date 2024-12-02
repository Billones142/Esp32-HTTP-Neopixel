#pragma once
#include <Adafruit_NeoPixel.h>

void solInit(Adafruit_NeoPixel &pixelsToChange);
void solStartTask();
void stopLuaTask();
void changeScript(const char *newScript);
bool isLuaWorking();