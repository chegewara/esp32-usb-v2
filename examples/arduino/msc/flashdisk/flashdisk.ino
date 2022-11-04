#include "Arduino.h"
#include "flashdisk.hpp"

using namespace esptinyusb;

USBflash disk;


void setup()
{
    Serial.begin(115200);

    if(disk.partition("/fat", "ffat"))
    {
        disk.onInquiry([](uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) -> void
        {
            const char vid[] = "ESP32-S3";
            const char pid[] = "flash disk";
            const char rev[] = "1.11";

            memcpy(vendor_id, vid, strlen(vid));
            memcpy(product_id, pid, strlen(pid));
            memcpy(product_rev, rev, strlen(rev));
        });
        if (!disk.begin())
            Serial.println("failed to begin flashdisk");
    }
}

void loop()
{
    delay(1000);
}
