#include <Arduino.h>
#include <luaNeopixel.h>
#include <sol/sol.hpp>
#include <lua.hpp>
#include <Adafruit_NeoPixel.h>

#define SOL_NO_THREAD_LOCAL 1
static sol::state lua;

static Adafruit_NeoPixel *pixelsLua = nullptr;

static TaskHandle_t luaTaskHandle;
static SemaphoreHandle_t luaFunctionUpdating = xSemaphoreCreateBinary();
static sol::protected_function lastestLuaLoopFunction;
static sol::protected_function wrapLuaScriptToFunction(const String &script);

static bool luaTaskExecuting = false; // not to be accesed directly, use isLuaWorking or setLuaWorking
static void setLuaWorking(bool newState);

// funtions for lua scripts
static void luaDelay(uint32_t delayMillis);
static void luaLoop(void *parameter);
static void luaSetPixelColor(uint16_t pixel, uint32_t color);
static uint32_t luaGetPixelColor(uint16_t pixel);
static void luaPrint(const char *message);
static uint32_t luaGetColor(uint8_t r, uint8_t g, uint8_t b);
static void luaShowPixels();
static void setPixelBrightness(uint8_t brigtness);
static void luaClearPixels();
static uint16_t luaGetPixelAmount();
static uint32_t colorWheel(byte pos);
static void luaSetPixelBrightness(uint8_t brightness);

void solInit(Adafruit_NeoPixel *pixelsToChange)
{
    xSemaphoreGive(luaFunctionUpdating);
    pixelsLua = pixelsToChange;
    // for print
    lua.open_libraries(sol::lib::math);

    // custom functions
    lua.set_function("delay", luaDelay);
    lua.set_function("print", luaPrint);
    lua.set_function("stopScript", stopLuaTask);

    lua.set_function("setPixelColor", luaSetPixelColor);
    lua.set_function("getPixelColor", luaGetPixelColor);
    lua.set_function("getColor", luaGetColor);
    lua.set_function("setPixelBrightness", luaSetPixelBrightness);
    lua.set_function("showPixels", luaShowPixels);
    lua.set_function("getPixelAmount", luaGetPixelAmount);
    lua.set_function("clearPixels", luaClearPixels);
    lua.set_function("colorWheel", colorWheel);
}

void startLuaTask()
{
    if (lastestLuaLoopFunction.valid())
    {
        Serial.println("Lua function invalid, skipping task creation.");
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
    }
}
void luaPrint(const char *message)
{
    Serial.println((String) "Lua script: " + message);
}

void stopLuaTask()
{
    if (isLuaWorking())
    {
        vTaskDelete(luaTaskHandle);
        setLuaWorking(false);
        luaTaskHandle = nullptr;
        Serial.println("Lua loop task stopped");
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
        if (xSemaphoreTake(luaFunctionUpdating, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            stopLuaTask();
            scriptNotChanged = false;
            lastestLuaLoopFunction = wrapLuaScriptToFunction(newScript);
            xSemaphoreGive(luaFunctionUpdating);
        }
    }
}

#pragma region functions for lua script
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
    if (delayMillis > 0)
    {
        TickType_t startTick = xTaskGetTickCount(); // Obtiene el tick actual
        TickType_t ticksToDelay = pdMS_TO_TICKS(delayMillis);

        // Espera hasta que pase el tiempo indicado
        vTaskDelayUntil(&startTick, ticksToDelay);
    }
}

void luaSetPixelBrightness(uint8_t brightness)
{
    pixelsLua->setBrightness(brightness);
}
#pragma endregion

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

    sol::protected_function loopFunc;
    try
    {
        // Execute the wrapped script to define the 'loop' function
        lua.safe_script(wrappedScript.c_str());
    }
    catch (const sol::error &e)
    {
        Serial.print("Lua script parsing error: ");
        Serial.println(e.what());
        lua.safe_script("function loop()\nprint(\"Lua script parsing error\")\nstopScript()\nend");
    }

    loopFunc = lua["loop"];
    // Retrieve the 'loop' function from Lua
    if (!loopFunc.valid())
    {
        Serial.println("Lua 'loop' function is not valid or undefined.");
        throw std::runtime_error("Lua 'loop' function is not valid or undefined.");
    }

    // Return the valid 'loop' function
    return loopFunc;
}

void luaLoop(void *parameter)
{
    sol::protected_function luaLoopFunction;
    if (xSemaphoreTake(luaFunctionUpdating, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
        luaLoopFunction = lastestLuaLoopFunction;
        xSemaphoreGive(luaFunctionUpdating);
    }
    else
    {
        xSemaphoreGive(luaFunctionUpdating);
        stopLuaTask();
    }

    while (true)
    {
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
    }
    stopLuaTask();
    return;
}