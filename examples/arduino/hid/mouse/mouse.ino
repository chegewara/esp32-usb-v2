#include "Arduino.h"

#include "usb_mouse.hpp"

using namespace esptinyusb;
USBmouse mouse;


void setup()
{
    Serial.begin(115200);

    mouse.init();
    mouse.begin(1);
}


void loop()
{
    mouse.sendReport(0, 5, 5);
    delay(10);
    mouse.sendReport(0);
    delay(250);
}
