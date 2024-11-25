#include "config.h"
#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


#include <wifiLib.h>
#include <httpServer.h>
#include <otaLib.h>
#include <neopixel.h>

#pragma message("\nWiFi SSID: " WIFI_SSID "\nWiFi Password: " WIFI_PASSWORD)


void setup() {
    Serial.begin(115200U);
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0U);
    
    wifi_Init(WIFI_SSID, WIFI_PASSWORD);

    OTA_Init(OTA_PASSWORD, 5250U); // Comes after the wifi config (needs the ip)

    wifi_print_status();

    neopixel_Init(NUMPIXELS, PIN);
    initHttpServer();
}

void loop() {
    arduino_OTA_Handle();
}

/*#include <neopixel.h>

#define PIN 23           // Pin GPIO para la señal de datos
#define NUMPIXELS 10

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(115200);
    pixels.begin();
}

void loop(){
    for (unsigned int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(100,100,100)); 
        Serial.println(i);
    }
    pixels.show();
    delay(1000);
    for (unsigned int i = 0; i < NUMPIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        Serial.println(i);
    }
    pixels.show();
    delay(1000);
}*/