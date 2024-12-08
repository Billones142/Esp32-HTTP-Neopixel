#include <neopixel.h>

Adafruit_NeoPixel PROGMEM pixels;

static JsonDocument parseJson(String jsonString);
static void printFSFiles(fs::FS &fs);
static void changePixelManual(Adafruit_NeoPixel &pixelStrip, JsonArray &colours);
static void fsSaveScript(fs::FS &fs, const char *scriptToSave, const char *scriptName);
static void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
/**
 * Converts the name of a Lua script to the name to be accessed in a file system.
 */
String toNameOfSavedFsLua(String scriptName);

void printFSFiles(fs::FS &fs)
{
    Serial.println("Archivos: ");

    // Abre el directorio raíz
    File root = fs.open("/");
    if (!root || !root.isDirectory())
    {
        root.close();
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
    file.close();
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
        printFSFiles(LittleFS);
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
        if (index == -1)
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

JsonDocument parseJson(String jsonString)
{
    JsonDocument doc;
    // Parse JSON object
    DeserializationError error = deserializeJson(doc, jsonString);
    if (error)
    {
        String errorMessage = (String) "Error parsing json:\n" + error.c_str();
        throw std::runtime_error(errorMessage.c_str());
    }
    return doc;
}

void processJsonToNeopixelStatic(Adafruit_NeoPixel &pixelStrip, String jsonString)
{
    auto doc = parseJson(jsonString);

    if (!isLuaWorking())
    {
        if (!doc["colours"].is<JsonArray>())
        {
            throw std::runtime_error("Colours is not an array or does not exist");
        }

        JsonArray colours = doc["colours"];

        if (!(doc["colours"].as<JsonArray>().size() > 0))
        {
            throw std::runtime_error("Colours array empty");
        }

        changePixelManual(pixels, colours);
    }
}


String toNameOfSavedFsLua(String scriptName)
{
    return (String) "/" + scriptName + ".lua";
}

void fsSaveScript(fs::FS &fs, const char *scriptToSave, const char *scriptName)
{
    String filename = toNameOfSavedFsLua(scriptName);
    File file = fs.open(filename.c_str(), "w");
    size_t savedSize = file.print(scriptToSave);
    file.close();
    if (savedSize)
    {
        Serial.println((String) "script written" + scriptName);
    }
    else
    {
        throw std::runtime_error("Script write failed");
    }
}

String fsReadScript(fs::FS &fs, String scriptName)
{
    if (scriptName.isEmpty())
    {
        throw std::runtime_error("Script name empty");
    }

    String filename = toNameOfSavedFsLua(scriptName);
    File file = fs.open(filename, "r");
    if (!file)
    {
        throw std::runtime_error("Failed to open file");
    }
    else if (file.isDirectory())
    {
        throw std::runtime_error("Path was a directory");
    }

    String fileContent = file.readString();
    file.close();
    return fileContent;
}

JsonDocument getSavedLuaScripts(bool retrieveScriptContent)
{
    return getSavedLuaScripts(LittleFS, retrieveScriptContent);
}

JsonDocument getSavedLuaScripts(fs::FS &fs, bool retrieveScriptContent)
{
    JsonDocument doc;
    JsonArray luaScripts = doc.to<JsonArray>();
    File root = fs.open("/");
    if (!root)
    {
        throw std::runtime_error("failed to open directory");
    }
    if (!root.isDirectory())
    {
        throw std::runtime_error("path not a directory");
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
            String filename = file.name();
            if (filename.endsWith(".lua"))
            {
                JsonObject scriptToAdd = luaScripts.createNestedObject();
                scriptToAdd["name"] = filename;

                if (retrieveScriptContent)
                {
                    String content = file.readString();
                    scriptToAdd["content"] = content;
                }
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
        throw std::runtime_error("failed to open directory");
    }
    if (!root.isDirectory())
    {
        throw std::runtime_error("path not a directory");
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

void processJsonToNeopixelScript(Adafruit_NeoPixel &pixelStrip, String jsonString)
{
    auto doc = parseJson(jsonString);
    Serial.print("processJsonToNeopixelScript json:\n");
    Serial.println(jsonString);

    bool applySavedScript = doc["applySavedScript"].as<bool>() | false;
    bool saveScript = doc["saveScript"].as<bool>() | false;
    bool applyScript = doc["applyScript"].as<bool>() | false;
    String scriptName = doc["scriptName"].as<String>();
    String luaScript = doc["luaScript"].as<String>();

    if (applySavedScript)
    {
        try
        {
            String retrievedScript = fsReadScript(LittleFS, scriptName.c_str());
            changeScript(retrievedScript.c_str());
        }
        catch (const std::exception &e)
        {
            String errorMessage = "Failed apply saved script:\n" + String(e.what());
            throw std::runtime_error(errorMessage.c_str());
        }
        return;
    }

    if (saveScript)
    {
        fsSaveScript(LittleFS, luaScript.c_str(), scriptName.c_str());
    }

    if (applyScript)
    {
        changeScript(luaScript.c_str());
    }
}