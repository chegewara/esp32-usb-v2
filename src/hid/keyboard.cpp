#include "../usb_keyboard.hpp"

#if CONFIG_TINYUSB
#if CFG_TUD_HID

namespace esptinyusb
{

    bool USBkeyboard::begin(uint8_t eps, uint8_t *desc, size_t len)
    {
        if (eps < 0)
            return true;
        auto intf = addInterface();
        ifIdx = intf->claimInterface();
        intf->addEndpoint(eps);
#ifdef CONFIG_TINYUSB_DESC_HID_STRING
        stringIndex = addString(CONFIG_TINYUSB_DESC_HID_STRING, -1);
#endif
        uint8_t tmp[] = {TUD_HID_DESCRIPTOR((uint8_t)intf->ifIdx, (uint8_t)stringIndex, HID_ITF_PROTOCOL_KEYBOARD, _desc_hid_report.size(), (uint8_t)(0x80 | intf->endpoints.at(0)->epId), _report_len, 1)};

        if (len > 0)
            intf->setDesc(desc, len);
        else
            intf->setDesc(tmp, sizeof(tmp));

        insertDevice();
        return true;
    }

} // namespace esptinyusb

#endif
#endif
