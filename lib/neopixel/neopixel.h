#pragma once
#include <config.h>

#include <Adafruit_NeoPixel.h>
extern Adafruit_NeoPixel pixels;

void neopixel_Init(uint16_t pixelAmount, uint16_t pin);

enum NeopixelJsonStatus : int8_t
{
    UKNOWNN_ERROR = -1,
    JSON_OK = 0,
    JSON_PARSE_ERROR,
    NO_PROPERTY_COLOURS,
    COLOURS_ARRAY_EMPTY,
};

NeopixelJsonStatus processJsonToNeopixel(Adafruit_NeoPixel &pixels, String jsonString);
