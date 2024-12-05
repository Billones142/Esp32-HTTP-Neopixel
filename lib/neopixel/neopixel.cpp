#include <neopixel.h>
#include <luaNeopixel.h>

Adafruit_NeoPixel pixels;

void printLittleFSFiles();
NeopixelJsonStatus processJsonToNeopixelScript(Adafruit_NeoPixel &pixelStrip, String jsonString);
void changePixelManual(Adafruit_NeoPixel &pixelStrip, JsonArray &colours);
NeopixelJsonStatus processJsonToNeopixelStatic(Adafruit_NeoPixel &pixelStrip, String jsonString);
NeopixelJsonStatus processJsonToNeopixelScript(Adafruit_NeoPixel &pixelStrip, String jsonString);
void littleFsSaveScript(fs::FS &fs, const char *scriptToSave, const char *scriptName);
void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

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
        Serial.println("LittleFS started successfully");
        printLittleFSFiles();
    }
    else
    {
        Serial.println("LittleFS error, reformating");
        LittleFS.format();
    }
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
        String filename = (String)scriptName + ".lua";
        File file = LittleFS.open(filename.c_str(), "w");
        if (file.print(scriptToSave))
        {
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
    String filename = (String)scriptName + ".lua";
    File file = LittleFS.open(filename.c_str(), "r");
    if (!file || file.isDirectory())
    {
        Serial.println("- failed to open file for reading");
        return "";
    }

    String fileContent = file.readString();
    file.close();
    return fileContent;
}

JsonArray getSavedLuaScripts()
{
    return getSavedLuaScripts(LittleFS);
}

JsonArray getSavedLuaScripts(fs::FS &fs)
{
    JsonArray luaScripts;
    File root = fs.open("/");
    if (!root)
    {
        Serial.println("- failed to open root directory");
        return luaScripts;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return luaScripts;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
        }
        else
        {
            String filename= file.name();
            if (filename.endsWith(".lua"))
            {
                JsonObject scriptToAdd;
                scriptToAdd["name"]= filename;
                scriptToAdd["content"]= file.readString();
                luaScripts.add(scriptToAdd);
            }
        }
        file = root.openNextFile();
    }
    return luaScripts;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.path(), levels - 1);
            }
        }
        else
        {
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
    NeopixelJsonStatus status = NeopixelJsonStatus::UKNOWNN_ERROR;
    JsonDocument doc;
    // Parse JSON object

    DeserializationError error = deserializeJson(doc, jsonString);
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        status = NeopixelJsonStatus::JSON_PARSE_ERROR;
    }

    bool applySavedScript = doc["applySavedScript"].as<bool>();
    bool saveScript = doc["saveScript"].as<bool>();
    bool applyScript = doc["applyScript"].as<bool>();
    String scriptName = doc["scriptName"].as<String>();
    String luaScript = doc["luaScript"].as<String>();
    Serial.print("applySavedScript: ");
    Serial.println(applySavedScript);

    Serial.print("saveScript: ");
    Serial.println(saveScript);

    Serial.print("applyScript: ");
    Serial.println(applyScript);

    Serial.print("scriptName: ");
    Serial.println(scriptName);

    Serial.print("luaScript: ");
    Serial.println(luaScript);

    if (applySavedScript && (status == NeopixelJsonStatus::UKNOWNN_ERROR))
    {
        changeScript(littleFsReadScript(LittleFS, scriptName.c_str()).c_str());
        status = NeopixelJsonStatus::JSON_OK;
    }

    if (saveScript && (status == NeopixelJsonStatus::UKNOWNN_ERROR))
    {
        littleFsSaveScript(LittleFS, luaScript.c_str(), scriptName.c_str());
        status = status;
    }

    if (applyScript && (status == NeopixelJsonStatus::UKNOWNN_ERROR))
    {
        changeScript(luaScript.c_str());
        status = NeopixelJsonStatus::JSON_OK;
    }

    return status;
}