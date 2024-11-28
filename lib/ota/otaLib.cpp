#include <otaLib.h>
#ifndef WIFI
#define WIFI
#include <WiFi.h>
#endif

void OTA_Init(const char *password, uint16_t port)
{
    ArduinoOTA.setPort(port);
    ArduinoOTA.setPassword(password);

    ArduinoOTA.onStart([]()
                       {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        Serial.println("OTA start. Type: " + type); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\nOTA Complete"); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
        Serial.printf("OTA Error [%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Authentication Error");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Error starting");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connection Error");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Error receiving");
        else if (error == OTA_END_ERROR) Serial.println("Error ending"); });

    // Inicia OTA
    ArduinoOTA.begin();

    Serial.println("Listo para OTA");
}
