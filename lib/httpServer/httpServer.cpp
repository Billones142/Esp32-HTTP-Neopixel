#include <httpServer.h>
#include <config.h>

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <neopixel.h>

#define HTML_URL "https://billones142.github.io/Esp32-HTTP-Neopixel/data/index.html"
// #include <WiFiServerSecure.h>

AsyncWebServer server(80);
// WiFiServerSecure server(443);

// functions used only inside this lib
void setResponse(String &message, int jsonResponse);
void sendJsonResponse(AsyncWebServerRequest *request, const String &status, const String &message);

void sendJsonResponse(AsyncWebServerRequest *request, const String &status, const String &message)
{
    String content = "{\"status\":\"" + status + "\",\"message\":\"" + message + "\"}";
    request->send("Success" ? 200 : 500, "application/json", content);
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
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        response->addHeader("Access-Control-Allow-Headers", "Content-Type");
        request->send(response); });

    // Serve a simple HTML page
    server.on("/getNumpixels", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String message= (String)"{\"message\":\"" + pixels.numPixels() + " pixel/s\"}";
        request->send(200, "text/html", 
        message.c_str()); });

    server.on("/getSavedScripts", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        JsonDocument bodyResponse;
        bodyResponse["status"]= "Success";
        bodyResponse["scripts"]= getSavedLuaScripts();
        if (bodyResponse["scripts"].size() > 0)
        {
            bodyResponse["message"]= "Scripts send";
        }
        else
        {
            bodyResponse["message"]= "No scripts found";
        }
        
        

        request->send(200, "application/json", bodyResponse.as<String>());});

    server.on("/setcolor", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        String status;
        String message;
        
        String requestData = String((char*)data, len);
        
        int jsonResponse = processJsonToNeopixelStatic(pixels, requestData.c_str());

        if (jsonResponse == NeopixelJsonStatus::JSON_OK)
        {
            status= "Success";
        }
        else
        {
            status= "Error";
        }

        setResponse(message, jsonResponse);
       
        String content= (String)"{\"status\":\"" + status + "\",\"message\":\"" + message + "\"}";
        sendJsonResponse(request, status, message); });

    server.on("/setScript", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        String status;
        String message;
        
        String requestData = String((char*)data, len);
        
        int jsonResponse = processJsonToNeopixelScript(pixels, requestData.c_str());

        if (jsonResponse == NeopixelJsonStatus::JSON_OK)
        {
            status= "Success";
        }
        else
        {
            status= "Error";
        }

        setResponse(message, jsonResponse);
       
        String content= (String)"{\"status\":\"" + status + "\",\"message\":\"" + message + "\"}";
        sendJsonResponse(request, status, message); });

    server.begin();
    Serial.println("HTTP server started");
}

void setResponse(String &message, int jsonResponse)
{
    switch (jsonResponse)
    {
    case NeopixelJsonStatus::JSON_OK:
        message = "Neopixel strip updated";
        break;

    case NeopixelJsonStatus::NO_PROPERTY_COLOURS:
        message = "Colours property not found in json";
        break;

    case NeopixelJsonStatus::JSON_PARSE_ERROR:
        message = "Json parse error";
        break;

    case NeopixelJsonStatus::COLOURS_ARRAY_EMPTY:
        message = "Colours array empty in json";
        break;

    case NeopixelJsonStatus::UKNOWNN_ERROR:
        message = "Unknown json error";
        break;

    default:
        message = "Unknown led status";
        break;
    }
}