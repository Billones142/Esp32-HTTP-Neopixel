#include <otaLib.h>
#include <ArduinoOTA.h>
#ifndef WIFI
    #define WIFI
    #include <WiFi.h>
#endif

void arduino_OTA_Handle(){
    ArduinoOTA.handle();
}

void OTA_Init(const char* password, uint16_t port){
    // Establece manualmente el hostname si no existe setHostname
    ArduinoOTA.setPort(port);  // Opcional: cambia el puerto predeterminado
    ArduinoOTA.setPassword(password);  // Este método debería estar disponible

    // Configuración de eventos (opcional)
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("Inicio de OTA. Tipo: " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA Finalizado");
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error OTA [%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Error de autenticación");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Error al iniciar");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Error de conexión");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Error al recibir");
        else if (error == OTA_END_ERROR) Serial.println("Error al finalizar");
    });

    // Inicia OTA
    ArduinoOTA.begin();

    Serial.println("Listo para OTA");
}
