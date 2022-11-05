#include "Arduino.h"
#include "flashdisk.hpp"

using namespace esptinyusb;

USBflash disk;


void setup()
{
    Serial.begin(115200);


    if (!disk.begin())
        Serial.println("failed to begin cdc");

    disk.partition("/fat", "ffat");

}

void loop()
{
    // Serial.println("test");
    delay(1000);
}
