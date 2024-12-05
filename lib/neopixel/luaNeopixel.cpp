#include <Arduino.h>
#include <luaNeopixel.h>
#include <sol/sol.hpp>
#include <lua.hpp>
#include <Adafruit_NeoPixel.h>

#define SOL_NO_THREAD_LOCAL 1
sol::state lua;

Adafruit_NeoPixel *pixelsLua = nullptr;

TaskHandle_t luaTaskHandle;
SemaphoreHandle_t luaScriptExecuting = xSemaphoreCreateBinary();
String luaScript = "";
bool newLuaScript = false;
bool luaTaskExecuting = false;

void solStartTask();
void setLuaWorking(bool newState);
void luaDelay(uint32_t delayMillis);
void luaLoop(void *parameter);

// funtions for lua scripts
void luaSetPixelColor(uint16_t pixel, uint32_t color);
uint32_t luaGetPixelColor(uint16_t pixel);
void luaPrint(const char *message);
uint32_t luaGetColor(uint8_t r, uint8_t g, uint8_t b);
void luaShowPixels();
void setPixelBrightness(uint8_t brigtness);
void luaClearPixels();
uint16_t luaGetPixelAmount();
uint32_t colorWheel(byte pos);

void solInit(Adafruit_NeoPixel *pixelsToChange)
{
    xSemaphoreGive(luaScriptExecuting);
    pixelsLua = pixelsToChange;
    // for print
    lua.open_libraries(sol::lib::base);

    // custom functions
    lua.set_function("delay", luaDelay);
    lua.set_function("print", luaPrint);

    lua.set_function("setPixelColor", luaSetPixelColor);
    lua.set_function("getPixelColor", luaGetPixelColor);
    lua.set_function("getColor", luaGetColor);
    lua.set_function("setPixelBrightness", luaGetColor);
    lua.set_function("showPixels", luaShowPixels);
    lua.set_function("getPixelAmount", luaGetPixelAmount);
    lua.set_function("clearPixels", luaClearPixels);
    lua.set_function("colorWheel", colorWheel);
}

void solStartTask()
{
    if (luaScript.isEmpty())
    {
        Serial.println("Empty Lua script, skipping execution.");
        return;
    }
    if (!isLuaWorking())
    {
        // Proceed with task creation
        xTaskCreatePinnedToCore(
            luaLoop,
            "LuaLoop",
            8192,
            NULL,
            1,
            &luaTaskHandle,
            0);
        setLuaWorking(true);
        xSemaphoreGive(luaScriptExecuting);
    }
}
void luaPrint(const char *message)
{
    Serial.println((String) "Lua script: " + message);
}

void stopLuaTask()
{
    while (xSemaphoreTake(luaScriptExecuting, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        if (isLuaWorking())
        {
            vTaskDelete(luaTaskHandle);
            setLuaWorking(false);
            luaTaskHandle = nullptr;
            Serial.println("Lua loop task stopped");
        }
    }
}

bool isLuaWorking()
{
    if (luaTaskHandle)
    {
        return luaTaskExecuting;
    }
    else
    {
        return false;
    }
}

void setLuaWorking(bool newState)
{
    luaTaskExecuting = newState;
}

void changeScript(const char *newScript)
{
    bool scriptNotChanged = true;
    while (scriptNotChanged)
    {
        if (xSemaphoreTake(luaScriptExecuting, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            luaScript = newScript;
            newLuaScript = true;
            xSemaphoreGive(luaScriptExecuting);
            stopLuaTask();
            solStartTask();
            scriptNotChanged = false;
        }
    }
}

void luaSetPixelColor(uint16_t pixel, uint32_t color)
{
    pixelsLua->setPixelColor(pixel, color);
}

uint32_t luaGetPixelColor(uint16_t pixel)
{
    return pixelsLua->getPixelColor(pixel);
}

uint32_t luaGetColor(uint8_t r, uint8_t g, uint8_t b)
{
    return pixelsLua->Color(r, g, b);
}

void setPixelBrightness(uint8_t brigtness)
{
    pixelsLua->setBrightness(brigtness);
}

uint16_t luaGetPixelAmount()
{
    return pixelsLua->numPixels();
}

void luaShowPixels()
{
    pixelsLua->show();
}

void luaClearPixels()
{
    pixelsLua->clear();
}

uint32_t colorWheel(byte pos)
{
    if (pos < 85)
    {
        return pixelsLua->Color(pos * 3, 255 - pos * 3, 0); // Rojo a verde
    }
    else if (pos < 170)
    {
        pos -= 85;
        return pixelsLua->Color(255 - pos * 3, 0, pos * 3); // Verde a azul
    }
    else
    {
        pos -= 170;
        return pixelsLua->Color(0, pos * 3, 255 - pos * 3); // Azul a rojo
    }
}

// Implementa la función delay para Lua
void luaDelay(uint32_t delayMillis)
{
    TickType_t startTick = xTaskGetTickCount(); // Obtiene el tick actual
    TickType_t ticksToDelay = pdMS_TO_TICKS(delayMillis);

    // Espera hasta que pase el tiempo indicado
    vTaskDelayUntil(&startTick, ticksToDelay);
}

sol::protected_function wrapLuaScriptToFunction(const String &script)
{
    // Ensure the Lua script is not empty
    if (script.isEmpty())
    {
        Serial.println("Provided Lua script is empty.");
        return sol::protected_function();
    }

    // Wrap the script inside a `loop` function
    String wrappedScript = "function loop()\n" + script + "\nend";

    try
    {
        // Execute the wrapped script to define the 'loop' function
        lua.safe_script(wrappedScript.c_str());

        // Retrieve the 'loop' function from Lua
        sol::protected_function loopFunc = lua["loop"];
        if (!loopFunc.valid())
        {
            Serial.println("Lua 'loop' function is not valid or undefined.");
            return sol::protected_function();
        }

        // Return the valid 'loop' function
        return loopFunc;
    }
    catch (const sol::error &e)
    {
        Serial.print("Lua script parsing error: ");
        Serial.println(e.what());
    }
    catch (...)
    {
        Serial.println("Unknown error while parsing Lua script.");
    }

    // Return an empty function on failure
    return sol::protected_function();
}

void luaLoop(void *parameter)
{
    sol::protected_function luaLoopFunction = wrapLuaScriptToFunction(luaScript);
    newLuaScript = false;

    while (!newLuaScript)
    {
        xSemaphoreTake(luaScriptExecuting, portMAX_DELAY);
        try
        {
            auto result = luaLoopFunction();
            // Serial.println(result.get<std::string>().c_str());
        }
        catch (const sol::error &e)
        {
            Serial.print("Lua runtime error: ");
            Serial.println(e.what());
            stopLuaTask();
            return;
        }
        catch (...)
        {
            Serial.println("Unknown error in Lua script.");
            stopLuaTask();
            return;
        }
        xSemaphoreGive(luaScriptExecuting);
    }
    stopLuaTask();
    return;
}