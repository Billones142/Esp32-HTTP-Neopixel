#include <httpServer.h>
#include <config.h>
#include <LittleFS.h>

#include <ESPAsyncWebServer.h>
#include <neopixel.h>

#include <neopixel.h>

AsyncWebServer server(80);

String htmlString;

void setResponse(String &message, int jsonResponse);

void initHttpServer()
{
    // Inicializa LittleFS y verifica si se monta correctamente
    if (!LittleFS.begin()){
        File file = LittleFS.open("/index.html", "r");
        if (!file)
        {
            htmlString = file.readString();
        }
        file.close();
    }
    else
    {
        Serial.println("Error al montar LittleFS");
        LittleFS.format();
        htmlString = "<h1> Html error </h1>";
    }

    // Manejador genérico para agregar encabezados CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    
    // Servir el archivo HTML
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, "text/html", htmlString.c_str()); });

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

    server.on("/setcolor", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        String status;
        String message;
        
        String requestData = String((char*)data, len);
        
        int jsonResponse = processJsonToNeopixel(pixels, requestData.c_str());

        if (jsonResponse == NeopixelJsonStatus::JSON_OK)
        {
            status= "Success";
        } else {
            status= "Error";
        }

        setResponse(message, jsonResponse);
       
        String content= (String)"{\"status\":\"" + status + "\",\"message\":\"" + message + "\"}";
        //Serial.println(content);
        auto statusCode= status == "Success"?200:500;
        request->send(statusCode,"application/json", content.c_str()); });

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
        message = "Uknown json error";
        break;

    default:
        message = "Uknown led status";
        break;
    }
}