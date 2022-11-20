#include "Arduino.h"

#include "usb_mouse.hpp"
#include "usb_hid.hpp"

#define USB_COMPOSITE_DEVICE 1
using namespace esptinyusb;
#ifdef USB_COMPOSITE_DEVICE
HIDdevice hid;
#else
USBmouse mouse;
#endif

uint8_t desc_hid_report1[] =
{
  TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(1)),
};
uint8_t desc_hid_report2[] =
{
  TUD_HID_REPORT_DESC_GAMEPAD ( HID_REPORT_ID(6)),
};
uint8_t desc_hid_report3[] =
{
  TUD_HID_REPORT_DESC_CONSUMER( HID_REPORT_ID(5)),
};
uint8_t desc_hid_report4[] =
{
  TUD_HID_REPORT_DESC_GENERIC_INOUT(32, HID_REPORT_ID(2)),
};
uint8_t desc_hid_report5[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD( HID_REPORT_ID(4)),
};

void setup()
{
    Serial.begin(115200);

#ifdef USB_COMPOSED_DEVICE
    hid.addHidReport(desc_hid_report1, sizeof(desc_hid_report1), 1, 5);
    hid.addHidReport(desc_hid_report2, sizeof(desc_hid_report2), 6, 11);
    hid.addHidReport(desc_hid_report3, sizeof(desc_hid_report3), 5, 2);
    hid.addHidReport(desc_hid_report4, sizeof(desc_hid_report4), 2, 32);
    hid.addHidReport(desc_hid_report5, sizeof(desc_hid_report5), 4, 9);
    hid.begin(3, NULL);
#else
    mouse.init();
    mouse.begin(1, NULL);
#endif
}


void loop()
{
    // mouse.sendReport(0, 5, 5);
    // delay(10);
    // mouse.sendReport(0);
    delay(250);
}
