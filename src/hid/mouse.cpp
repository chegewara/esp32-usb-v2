#include "../usb_mouse.hpp"

namespace esptinyusb
{

    bool USBmouse::begin(uint8_t eps, uint8_t *desc, size_t len)
    {
        if (eps < 0)
            return true;
        auto intf = addInterface();
        ifIdx = intf->claimInterface();
        intf->addEndpoint(eps);
#ifdef CONFIG_TINYUSB_DESC_HID_STRING
        stringIndex = addString(CONFIG_TINYUSB_DESC_HID_STRING, -1);
#endif
        uint8_t tmp[] = {TUD_HID_DESCRIPTOR((uint8_t)intf->ifIdx, (uint8_t)stringIndex, HID_ITF_PROTOCOL_MOUSE, _desc_hid_report.size(), (uint8_t)(0x80 | intf->endpoints.at(0)->epId), _report_len, 10)};

        if (len > 0)
            intf->setDesc(desc, len);
        else
            intf->setDesc(tmp, sizeof(tmp));

        insertDevice();
        return true;
    }

} // namespace esptinyusb