#pragma once
#include <Adafruit_NeoPixel.h>

void solInit(Adafruit_NeoPixel *pixelsToChange);
void startLuaTask();
void stopLuaTask();
/**
 * interprets the new script in the string and starts the lua loop task
 */
void changeScript(const char *newScript);
/**
 * returns true if the lua loop task is executing
 */
bool isLuaWorking();