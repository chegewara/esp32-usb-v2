#include "Arduino.h"

#include "usb_keyboard.hpp"

using namespace esptinyusb;
USBkeyboard keyboard;


void setup()
{
    Serial.begin(115200);

    keyboard.init();
    keyboard.begin(3);
}


void loop()
{
    keyboard.sendKey(KEYBOARD_MODIFIER_LEFTSHIFT, HID_KEY_A, HID_KEY_B, HID_KEY_C);
    delay(10);
    keyboard.sendKey();
    delay(1000);
}
