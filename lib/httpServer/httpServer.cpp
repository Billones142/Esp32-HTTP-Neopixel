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
    /*if (LittleFS.begin()){
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
        //htmlString = "<h1> Html error </h1>";
    }*/
        htmlString = "<!DOCTYPE html>\n"
                     "<html lang=\"en\">\n"
                     "<head>\n"
                     "  <meta charset=\"UTF-8\">\n"
                     "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                     "  <title>ESP32 Neopixel</title>\n"
                     "</head>\n"
                     "<body>\n"
                     "  <h1>ESP32 Neopixel</h1>\n"
                     "  <form id=\"colorForm\">\n"
                     "        <div>\n"
                     "            <label for=\"index\">Índice:</label>\n"
                     "            <input type=\"number\" value=\"0\" id=\"index\" name=\"i\" min=\"0\" required>\n"
                     "            <input name=\"Color Picker\" id=\"colorValues\" type=\"color\"/>\n"
                     "        </div>\n"
                     "        <button type=\"button\" onclick=\"sendData()\">Enviar</button>\n"
                     "    </form>\n"
                     "\n"
                     "  <p id=\"response\"></p>\n"
                     "\n"
                     "  <script>\n"
                     "    const colorSelector= document.getElementById('colorValues');\n"
                     "\n"
                     "    let debounceTimer;\n"
                     "    colorSelector.addEventListener('input', () => {\n"
                     "        clearTimeout(debounceTimer); // Clear the previous timer\n"
                     "        debounceTimer = setTimeout(() => {\n"
                     "            sendData();\n"
                     "        }, 7);\n"
                     "\n"
                     "    });\n"
                     "\n"
                     "    function updateValue(color, value) {\n"
                     "            document.getElementById(`${color}Value`).textContent = value;\n"
                     "    }\n"
                     "\n"
                     "    function hexToRgb(hex) {\n"
                     "        const red = parseInt(hex.substring(1, 3), 16); // Extraer y convertir rojo\n"
                     "        const green = parseInt(hex.substring(3, 5), 16); // Extraer y convertir verde\n"
                     "        const blue = parseInt(hex.substring(5, 7), 16); // Extraer y convertir azul\n"
                     "        return { red, green, blue };\n"
                     "    }\n"
                     "\n"
                     "    function sendData() {\n"
                     "        const index = document.getElementById('index').value;\n"
                     "        const {red, green, blue}= hexToRgb(colorSelector.value)\n"
                     "\n"
                     "        const jsonPayload = {\n"
                     "            colours: [\n"
                     "            {\n"
                     "                i: parseInt(index),\n"
                     "                r: red,\n"
                     "                g: green,\n"
                     "                b: blue\n"
                     "            }\n"
                     "            ]\n"
                     "        };\n"
                     "\n"
                     "        fetch('http://192.168.0.130/setcolor', { // Reemplaza con la IP del ESP32\n"
                     "            method: 'POST',\n"
                     "            headers: {\n"
                     "            'Content-Type': 'application/json'\n"
                     "            },\n"
                     "            body: JSON.stringify(jsonPayload)\n"
                     "        })\n"
                     "        .then(response => response.json())\n"
                     "        .then(data => {\n"
                     "            document.getElementById('response').textContent = `Respuesta del ESP32: ${data.message}`;\n"
                     "        })\n"
                     "        .catch(error => {\n"
                     "            document.getElementById('response').textContent = `Error: ${error}`;\n"
                     "        });\n"
                     "    }\n"
                     "  </script>\n"
                     "</body>\n"
                     "</html>\n";

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
        
        int jsonResponse = processJsonToNeopixelStatic(pixels, requestData.c_str());

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

    server.on("/setScript", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              {
        String status;
        String message;
        
        String requestData = String((char*)data, len);
        
        int jsonResponse = processJsonToNeopixelScript(pixels, requestData.c_str());

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