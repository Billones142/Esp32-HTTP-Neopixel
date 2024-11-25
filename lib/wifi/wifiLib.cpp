#include <wifiLib.h>
#include <Arduino.h>
#ifndef WIFI
    #define WIFI
    #include <WiFi.h>
#endif

#include <config.h>

void wifi_Init(const char* ssid, const char* password){
    WiFi.begin(ssid, password);

    int wifiStatus = WiFi.status();
    while (wifiStatus != WL_CONNECTED) {
        wifiStatus = WiFi.status();
        delay(1000);
        String defaultMessage= "Connecting to WiFi... Code= ";
        String mensaje= defaultMessage + wifiStatus;
        Serial.println(mensaje);
    }
    Serial.println("Connected to WiFi");
}

void wifi_print_status() {
    // print the SSID of the network you're attached to:
    String message_ssid= (String)"SSID: " + WiFi.SSID();
    Serial.println(message_ssid);
       
    // print your WiFi shield's IP address:
    String message_ip_address= (String)"IP Address: " + WiFi.localIP();
    Serial.println(message_ip_address);

    // print the received signal strength:

    String message_signal_strenght= (String)"signal strength (RSSI): " + WiFi.RSSI() + " dBm";
    Serial.println(message_signal_strenght);
}