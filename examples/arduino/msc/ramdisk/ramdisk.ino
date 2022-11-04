#include "Arduino.h"
#include "ramdisk.hpp"

using namespace esptinyusb;

USBram disk;

uint8_t* buffer = nullptr;

void setup()
{
    Serial.begin(115200);

    buffer = (uint8_t*)malloc(200*1024);
    disk.setCapacity(2*200, 512);

    if(buffer)
        disk.partition(buffer);
    else
        Serial.println("failed to allocate ramdisk");

    if (!disk.begin())
        Serial.println("failed to begin ramdisk");
}

void loop()
{
    
    delay(10);
}
