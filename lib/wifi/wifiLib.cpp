#include <wifiLib.h>
#include <Arduino.h>
#ifndef WIFI
#define WIFI
#include <WiFi.h>
#endif

#include <config.h>

/*
 * Initialices the wifi service
 * @param ssid Name of the wifi you want to connect to
 * @param password password of the wifi to connect
 */
void wifi_Init(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);

    int wifiStatus = WiFi.status();
    while (wifiStatus != WL_CONNECTED)
    {
        wifiStatus = WiFi.status();
        delay(1000);
        Serial.println((String) "Connecting to WiFi... Code= " + wifiStatus);
    }
    Serial.println("Connected to WiFi");
}

void wifi_print_status()
{
    // print the SSID of the network you're attached to:
    Serial.println((String) "SSID: " + WiFi.SSID());

    // print your WiFi shield's IP address:
    Serial.println((String) "IP Address: " + WiFi.localIP().toString());

    // print the received signal strength:
    Serial.println((String) "signal strength (RSSI): " + WiFi.RSSI() + " dBm");
}