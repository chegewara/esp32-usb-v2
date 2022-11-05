#include "Arduino.h"
#include "ramdisk.hpp"

using namespace esptinyusb;

USBram disk;

uint8_t* buffer = nullptr;

void setup()
{
    Serial.begin(115200);

    buffer = (uint8_t*)malloc(2*1024*1024);

    if (!disk.begin())
        Serial.println("failed to begin cdc");

    if(buffer)
        disk.partition(buffer);
    else
        Serial.println("failed to allocate ramdisk");

    disk.setCapacity(2*2*1024, 512);
}

void loop()
{
    
    delay(10);
}
