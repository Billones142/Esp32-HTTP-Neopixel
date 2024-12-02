#include <Arduino.h>
#include <luaNeopixel.h>
#include <sol/sol.hpp>
#include <lua.hpp>
#include <Adafruit_NeoPixel.h>

#define SOL_NO_THREAD_LOCAL 1
sol::state lua;

Adafruit_NeoPixel *pixelsLua = nullptr;

TaskHandle_t luaTaskHandle;
String luaScript = "";
SemaphoreHandle_t scriptSemaphore;
bool newLuaScript = false;
bool luaScriptWorking = false;

void solStartTask();
void luaDelay(uint32_t delayMillis);
void luaSetPixelColor(int pixel, uint32_t color);
void luaPrint(const char *message);
uint32_t luaGetColor(uint8_t r, uint8_t g, uint8_t b);
void luaShowPixels();
void luaClearPixels();
uint32_t colorWheel(byte pos);
void luaLoop(void *parameter);

void solInit(Adafruit_NeoPixel &pixelsToChange)
{
    scriptSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(scriptSemaphore); // Inicializa el semáforo en estado disponible
    pixelsLua = &pixelsToChange;
    // for print
    lua.open_libraries(sol::lib::base);

    // custom function
    lua.set_function("delay", luaDelay);

    lua.set_function("setPixelColor", luaSetPixelColor);
    lua.set_function("luaGetColor", luaGetColor);
    lua.set_function("showPixels", luaShowPixels);
    lua.set_function("clearPixels", luaClearPixels);
    lua.set_function("colorWheel", colorWheel);
    lua.set_function("print", luaPrint);
}

void solStartTask()
{
    luaScriptWorking = true;
    Serial.println("asignando tarea");
    // Crea la tarea y la asigna al núcleo 0
    xTaskCreatePinnedToCore(
        luaLoop,        // Función a ejecutar
        "LuaTask",      // Nombre de la tarea
        32768,          // Tamaño del stack en bytes
        NULL,           // Parámetros para la tarea (ninguno en este caso)
        1,              // Prioridad de la tarea
        &luaTaskHandle, // Manejador de la tarea
        0               // Núcleo donde se ejecutará (1)
    );
    Serial.println("FIN: asignando tarea");
}
void luaPrint(const char *message){
    Serial.println(message);
}

void stopLuaTask()
{
    if (luaTaskHandle)
    {
        vTaskDelete(luaTaskHandle);
        luaTaskHandle = nullptr;
        luaScriptWorking = false;
    }
}

bool isLuaWorking()
{
    return luaScriptWorking;
}

void changeScript(const char *newScript)
{
    luaScript = newScript;
    newLuaScript = true;
    if (!luaTaskHandle)
    {
        solStartTask();
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

/*void luaDelay(uint32_t delayMillis)
{
    unsigned long finishTime = millis() + delayMillis;
    while (millis() < finishTime)
    {
#ifdef ESP32
        vTaskDelay(1);
#endif
    }
   //delay(delayMillis);
}*/
// Implementa la función delay para Lua
void luaDelay(uint32_t delayMilis)
{
    unsigned long finishTime = millis() + delayMilis;
    while (millis() < finishTime){}
}

void luaLoop(void *parameter)
{
    try
    {
        lua.script(luaScript.c_str());
        newLuaScript = false;
    }
    catch (const sol::error &e)
    {
        Serial.print("Lua error: ");
        Serial.println(e.what());
        vTaskDelete(NULL); // Termina la tarea en caso de error
        return;
    }

    Serial.println("empezando lua loop");
    while (!newLuaScript)
    {
        Serial.println("lua loop");
        try
        {
            lua.safe_script(luaScript.c_str()); // Llama a la función 'loop' de Lua
        }
        catch (const sol::error &e)
        {
            Serial.print("Lua runtime error in 'loop': ");
            Serial.println(e.what());
            break; // Sale del loop si hay un error
        }
        catch (const std::runtime_error &e)
        {
            Serial.print("Lua runtime unknown error in 'loop': ");
            Serial.println(e.what());
            break; // Sale del loop si hay un error
        }

        /*#ifdef ESP32
                vTaskDelay(1); // Permite que el sistema operativo libere el CPU
        #endif*/
    }
    Serial.print("ending luaLoop");
    vTaskDelete(NULL); // Finaliza la tarea cuando se detecta un nuevo script
}