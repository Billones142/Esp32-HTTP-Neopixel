#include <httpServer.h>
#include <config.h>

#include <ESPAsyncWebServer.h>
#include <neopixel.h>

#include <neopixel.h>

AsyncWebServer server(80);

void initHttpServer(){
    // Serve a simple HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", 
        "<h1>Hello, ESP32!</h1>");
    });

    server.on("/setcolor", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
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

        switch (jsonResponse) {
        case NeopixelJsonStatus::JSON_OK:
            message= "Neopixel strip updated";
            break;

        case NeopixelJsonStatus::NO_PROPERTY_COLOURS:
            message= "Colours property not found";
            break;

        case NeopixelJsonStatus::JSON_PARSE_ERROR:
            message= "Json parse error";
            break;

        case NeopixelJsonStatus::COLOURS_ARRAY_EMPTY:
            message= "Colour array empty";
            break;

        case NeopixelJsonStatus::UKNOWNN_ERROR :
            message= "Uknown error";
            break;        

        default:
            message= "Uknown led status";
            break;
        }
        
       
        String content= (String)"{\"status\": \"" + status + "\",\"message\": \"" + message + "\"}";
        //Serial.println(content);
        auto statusCode= status == "Success"?200:500;
        request->send(statusCode,"application/json", content.c_str());
    });

    server.begin();
    Serial.println("HTTP server started");
}