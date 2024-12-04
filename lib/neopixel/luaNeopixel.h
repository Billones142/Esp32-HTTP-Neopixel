#pragma once
#include <Adafruit_NeoPixel.h>

void solInit(Adafruit_NeoPixel *pixelsToChange);
void solStartTask();
void stopLuaTask();
void changeScript(const char *newScript);
/**
 * returns true if the lua loop task is executing
 */
bool isLuaWorking();