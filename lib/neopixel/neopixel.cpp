#include <neopixel.h>
#include <ArduinoJson.h>
#include <luaNeopixel.h>

Adafruit_NeoPixel pixels;

void neopixel_Init(uint16_t pixelAmount, uint16_t pin)
{
    pixels = Adafruit_NeoPixel(pixelAmount, pin, NEO_GRB + NEO_KHZ800);
    pixels.begin();
    pixels.clear();
    pixels.show();
    solInit(pixels);
}

void changePixelManual(Adafruit_NeoPixel &pixelStrip, JsonArray &colours)
{
    uint16_t pixelsChanged = 0;

    for (JsonObject colour : colours)
    {
        uint16_t index;
        uint8_t red, green, blue;
        if (colour["i"].is<JsonInteger>())
        {
            index = colour["i"].as<JsonInteger>();
        }
        else
            continue;
        if (colour["r"].is<JsonInteger>())
        {
            red = colour["r"].as<JsonInteger>();
        }
        else
            continue;
        if (colour["g"].is<JsonInteger>())
        {
            green = colour["g"].as<JsonInteger>();
        }
        else
            continue;
        if (colour["b"].is<JsonInteger>())
        {
            blue = colour["b"].as<JsonInteger>();
        }
        else
            continue;

        pixelStrip.setPixelColor(index, pixelStrip.Color(red, green, blue));
        pixelsChanged++;
    }

    if (pixelsChanged > 0)
    {
        stopLuaTask();
        pixelStrip.show();
    }
}

NeopixelJsonStatus processJsonToNeopixelStatic(Adafruit_NeoPixel &pixelStrip, String jsonString)
{
    JsonDocument doc;
    // Parse JSON object
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return JSON_PARSE_ERROR;
    }

    
    if (!isLuaWorking())
    {
        if (!doc["colours"].is<JsonArray>())
        {
            return NO_PROPERTY_COLOURS;
        }

        JsonArray colours = doc["colours"];

        if (!(doc["colours"].as<JsonArray>().size() > 0))
        {
            return COLOURS_ARRAY_EMPTY;
        }

        changePixelManual(pixels, colours);
    }

    return JSON_OK;
}

NeopixelJsonStatus processJsonToNeopixelScript(Adafruit_NeoPixel &pixelStrip, String jsonString)
{
    JsonDocument doc;
    // Parse JSON object
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return JSON_PARSE_ERROR;
    }

    if (doc["luaScript"].is<JsonString>())
    {
        Serial.println(doc["luaScript"].as<JsonString>().c_str());
        changeScript(doc["luaScript"].as<JsonString>().c_str());
    }
}