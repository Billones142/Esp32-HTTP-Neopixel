#include <mdnsInit.h>
#include <ESPmDNS.h>

void mDNS_Init(const char *hostname, uint16_t otaPort)
{
    // Initialize mDNS
    if (!MDNS.begin(hostname))
    {
        Serial.println("Error starting mDNS");
        return;
    }
    Serial.println("mDNS responder started");

    // Add an HTTP service (optional)
    MDNS.addService("http", "tcp", 80);

    // Add an OTA service (optional)
    MDNS.addService("ota", "tcp", otaPort);
}