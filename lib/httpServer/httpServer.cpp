#include <httpServer.h>
#include <config.h>

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <neopixel.h>

#define HTML_URL "https://billones142.github.io/Esp32-HTTP-Neopixel/data/index.html"

AsyncWebServer server(80);

// typedef std::function<void(AsyncWebServerRequest* request)> JsonProcess;
typedef std::function<void(Adafruit_NeoPixel &pixelStrip, String jsonString)> JsonProcess;
void processJsonBase(JsonProcess processJsonFunction, AsyncWebServerRequest *request, String requestData, String succesMessage)
{
    JsonDocument response;

    try
    {
        processJsonFunction(pixels, requestData.c_str());
        response["status"] = "Success";
        response["message"] = succesMessage;
    }
    catch (const std::runtime_error &e)
    {
        response["status"] = "Error";
        response["message"] = e.what();
    }
    catch (...)
    {
        response["status"] = "Error";
        response["message"] = "Unknown error";
    }

    request->send(200, "application/json", response.as<String>());
}

void initHttpServer()
{
    // Manejador genérico para agregar encabezados CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    // Redirect to html on url defined in config
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->redirect(HTML_URL); });

    // Manejador para solicitudes OPTIONS
    server.on("/*", HTTP_OPTIONS, [](AsyncWebServerRequest *request)
              {
        AsyncWebServerResponse *response = request->beginResponse(204); // No Content
        /*response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        response->addHeader("Access-Control-Allow-Headers", "Content-Type");*/
        request->send(response); });

    // Serve a simple HTML page
    server.on("/getNumpixels", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        JsonDocument bodyResponse;
        bodyResponse["message"]= (String)pixels.numPixels() + " pixel/s";
        request->send(200, "text/html", bodyResponse.as<String>()); });

    server.on("/getSavedScripts", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        JsonDocument bodyResponse;
        bodyResponse["status"]= "Success";
        try
        {
            bodyResponse["scripts"]= getSavedLuaScripts(true);
        }
        catch(...)
        {
            bodyResponse["scripts"]= "failed";
        }
        

        if (bodyResponse["scripts"].size() > 0)
        {
            bodyResponse["message"]= "Scripts send";
        }
        else
        {
            bodyResponse["message"]= "No scripts found";
        }

        request->send(200, "application/json", bodyResponse.as<String>()); });

    server.on("/setcolor", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        stopLuaTask();
        String requestData = String((char*)data, len);
        processJsonBase(processJsonToNeopixelStatic, request, requestData, "Static colour applied"); });

    server.on("/setScript", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        String requestData = String((char*)data, len);
        processJsonBase(processJsonToNeopixelScript, request, requestData, "Script applied"); });

    server.begin();
    Serial.println("HTTP server started");
}