#include "Arduino.h"
#include "usb_cdc.hpp"

using namespace esptinyusb;

USBCDC cdc;

void setup()
{
    Serial.begin(115200);

    if (!cdc.begin())
        printf("failed to begin cdc\n");

    cdc.onData([](){
        printf("new data\n");
    });

}

void loop()
{
    while(Serial.available())
    {                             // If anything comes in Serial (USB),
        cdc.write(Serial.read());
    }

    while(cdc.available())
    {                             // If anything comes in Serial1 (pins 0 & 1)
        Serial.write(cdc.read()); // read it and send it out Serial (USB)
    }

    delay(10);
}
