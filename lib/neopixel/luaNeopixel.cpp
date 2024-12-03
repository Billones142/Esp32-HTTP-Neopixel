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
void luaDelay(uint32_t delayMillis);
void luaLoop(void *parameter);

void luaSetPixelColor(int pixel, uint32_t color);
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
        luaTaskExecuting = true;
        xSemaphoreGive(luaScriptExecuting);
    }
}
void luaPrint(const char *message)
{
    Serial.println((String) "Lua script: " + message);
}

void stopLuaTask()
{
    if (luaTaskExecuting)
    {
        vTaskDelete(luaTaskHandle);
        luaTaskExecuting = false;
        luaTaskHandle = nullptr;
    }
}

bool isLuaWorking()
{
    return (bool)luaTaskHandle;
}

void changeScript(const char *newScript)
{
    bool scriptNotChanged= true;
    while (scriptNotChanged)
    {
        if (xSemaphoreTake(luaScriptExecuting, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            luaScript = newScript;
            newLuaScript = true;
            stopLuaTask();
            solStartTask();
            xSemaphoreGive(luaScriptExecuting);
            scriptNotChanged= false;
        }
        
    }
    
}

void luaSetPixelColor(int pixel, uint32_t color)
{
    pixelsLua->setPixelColor(pixel, color);
}

uint32_t luaGetColor(uint8_t r, uint8_t g, uint8_t b)
{
    return pixelsLua->Color(r, g, b);
}

void setPixelBrightness(uint8_t brigtness){
    pixelsLua->setBrightness(brigtness);
}

uint16_t luaGetPixelAmount(){
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

void luaLoop(void *parameter)
{
    String luaScriptExec = luaScript;
    newLuaScript = false;
    while (!newLuaScript)
    {
        xSemaphoreTake(luaScriptExecuting, portMAX_DELAY);
        try
        {
            auto result = lua.safe_script(luaScriptExec.c_str());
            //Serial.println(result.get<std::string>().c_str());
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