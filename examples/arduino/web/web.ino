#include "Arduino.h"
#include "usb_webusb.hpp"

using namespace esptinyusb;

USBWebSerial web;
static bool connected = false;

void setup()
{
    Serial.begin(115200);

    if (!web.begin())
        Serial.println("failed to begin cdc");

    web.onData([]()
               { Serial.println("new data"); });

    web.onConnect([](bool is)
                  {
                    connected = is;
        if(is) Serial.println("host connected");
        else Serial.println("host disconnected"); });

    web.setVersion(0x110, 0x210); // 0x210 USB version to enable auto detect webSerial
    web.setVidPid(0xcafe, 0x100);

    web.landingPageURI("https://www.tinyusb.org/examples/webusb-serial/");
}

void loop()
{
    if (connected)
    {
        while (Serial.available())
        {
            char c = Serial.read();
            if (web.write(c) == 0)
                Serial.println("failed");
            Serial.write(c);
        }

        while (web.available())
        {
            int c = web.read();
            Serial.write((char)c);
        }
    }
    delay(10);
}
