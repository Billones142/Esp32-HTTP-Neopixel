#include "config.h"
#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


#include <wifiLib.h>
#include <httpServer.h>
#include <otaLib.h>
#include <neopixel.h>
#include <mdns.h>

#pragma message("\nWiFi SSID: " WIFI_SSID "\nWiFi Password: " WIFI_PASSWORD)


void setup() {
    Serial.begin(115200U);
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0U);
    
    wifi_Init(WIFI_SSID, WIFI_PASSWORD);
    mDNS_Init(WIFI_HOSTNAME, OTA_PORT);

    OTA_Init(OTA_PASSWORD, OTA_PORT); // Comes after the wifi config (needs the ip)

    wifi_print_status();

    neopixel_Init(NUMPIXELS, PIN);
    initHttpServer();

}

void loop() {
    ArduinoOTA.handle();
}