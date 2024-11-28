#include <neopixel.h>
#include <ArduinoJson.h>

Adafruit_NeoPixel pixels;

void neopixel_Init(uint16_t pixelAmount, uint16_t pin){
    pixels = Adafruit_NeoPixel(pixelAmount, pin, NEO_GRB + NEO_KHZ800);
    pixels.begin();
}

NeopixelJsonStatus processJsonToNeopixel(Adafruit_NeoPixel &pixels, String jsonString){
    JsonDocument doc;
    // Parse JSON object
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return JSON_PARSE_ERROR;
    }
    
    if(!doc["colours"].is<JsonArray>()){
        return NO_PROPERTY_COLOURS;
    }

    JsonArray colours = doc["colours"];

    if (!(doc["colours"].as<JsonArray>().size() > 0))
    {
        return COLOURS_ARRAY_EMPTY;
    }

    uint16_t pixelsChanged= 0;
    
    for (JsonObject colour : colours) {
        uint16_t index;
        uint8_t red, green, blue;
        if (colour["i"].is<JsonInteger>()){
            index = colour["i"].as<JsonInteger>();
        } else continue;
        if (colour["r"].is<JsonInteger>()){
            red = colour["r"].as<JsonInteger>();
        } else continue;
        if (colour["g"].is<JsonInteger>()){
            green = colour["g"].as<JsonInteger>();
        } else continue;
        if (colour["b"].is<JsonInteger>()){
            blue = colour["b"].as<JsonInteger>();
        } else continue;

        
        //Serial.println((String)"index: " + index + " red: " + red + " green: " + green + " blue: " + blue);
        pixels.setPixelColor(index, pixels.Color(red, green, blue));
        pixelsChanged++;
    }

    if (pixelsChanged > 0)
    {
        pixels.show();
    }
    //Serial.println("finished proccessing json");
    return JSON_OK;
}