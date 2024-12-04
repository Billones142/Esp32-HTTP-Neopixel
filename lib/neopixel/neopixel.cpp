#include <neopixel.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <luaNeopixel.h>

Adafruit_NeoPixel pixels;

void printLittleFSFiles()
{
    // Asegúrate de que LittleFS esté montado
    if (!LittleFS.begin())
    {
        Serial.println("Error: LittleFS no se pudo montar.");
        return;
    }

    Serial.println("Archivos en LittleFS:");

    // Abre el directorio raíz
    File root = LittleFS.open("/");
    if (!root || !root.isDirectory())
    {
        Serial.println("Error: No se pudo abrir el directorio raíz.");
        return;
    }

    // Itera a través de los archivos y directorios en el directorio raíz
    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("[Directorio] ");
        }
        else
        {
            Serial.print("[Archivo] ");
        }

        // Imprime el nombre del archivo/directorio
        Serial.print(file.name());

        // Si es un archivo, imprime el tamaño
        if (!file.isDirectory())
        {
            Serial.print(" - Tamaño: ");
            Serial.print(file.size());
            Serial.println(" bytes");
        }
        else
        {
            Serial.println();
        }

        // Abre el siguiente archivo/directorio
        file = root.openNextFile();
    }

    // Finaliza la sesión de LittleFS
    LittleFS.end();
}

void neopixel_Init(uint16_t pixelAmount, uint16_t pin)
{
    pixels = Adafruit_NeoPixel(pixelAmount, pin, NEO_GRB + NEO_KHZ800);
    pixels.begin();
    pixels.clear();
    pixels.show();
    if (LittleFS.begin())
    {
        Serial.println("LittleFS started succesfully");
        printLittleFSFiles();
    }
    else
    {
        Serial.println("LittleFS error, reformating");
        LittleFS.format();
    }
    LittleFS.end();
    solInit(&pixels);
}

void changePixelManual(Adafruit_NeoPixel &pixelStrip, JsonArray &colours)
{
    bool pixelsChanged = false;

    for (JsonObject colour : colours)
    {
        // Extraer valores directamente con valores predeterminados para simplificar
        uint16_t index = colour["i"] | -1; // Usa -1 para identificar índices no válidos
        uint8_t red = colour["r"] | 0;
        uint8_t green = colour["g"] | 0;
        uint8_t blue = colour["b"] | 0;

        // Verificar que el índice es válido
        if (index == (uint16_t)-1)
        {
            continue;
        }

        // Establecer el color del píxel
        pixelStrip.setPixelColor(index, pixelStrip.Color(red, green, blue));
        pixelsChanged = true;
    }

    // Actualizar el estado del NeoPixel si hubo cambios
    if (pixelsChanged)
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

void littleFsSaveScript(fs::FS &fs, const char *scriptToSave, const char *scriptName)
{
    if (LittleFS.begin())
    {
        String filename= (String)scriptName + ".lua";
        File file= LittleFS.open(filename.c_str(), "w");
        if(file.print(scriptToSave)){
            Serial.println("- script written");
        }
        else
        {
            Serial.println("- script write failed");
        }
        file.close();
    }
}

String littleFsReadScript(fs::FS &fs, const char *scriptName)
{
    String filename= (String)scriptName + ".lua";
    File file= LittleFS.open(filename.c_str(), "r");
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return "";
    }

    String fileContent= file.readString();
    file.close();
    return fileContent;
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
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
        return NeopixelJsonStatus::JSON_PARSE_ERROR;
    }
    
    if(doc["applySavedScript"] | false)
    {
        changeScript(littleFsReadScript(LittleFS, doc["scriptName"]).c_str());
        return NeopixelJsonStatus::JSON_OK;
    }

    if (doc["saveScript"] | false)
    {
        littleFsSaveScript(LittleFS, doc["luaScript"], doc["scriptName"]);
    }

    if (doc["applyScript"] && doc["luaScript"].is<JsonString>())
    {
        changeScript(doc["luaScript"].as<JsonString>().c_str());
        return NeopixelJsonStatus::JSON_OK;
    }
    return NeopixelJsonStatus::UKNOWNN_ERROR;
}