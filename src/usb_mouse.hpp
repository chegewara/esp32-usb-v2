#pragma once

#include "usb_hid.hpp"

namespace esptinyusb
{
    class USBmouse : public HIDdevice
    {
    private:
        // uint8_t _report[5] = {};
        uint8_t _report_id = 3;
        hid_mouse_report_t _report = {};

    public:
        using HIDdevice::HIDdevice;
        ~USBmouse() {}

        virtual bool begin(uint8_t eps, uint8_t* desc = nullptr, size_t len = 0) override
        {
            if(eps < 0) return true;
            auto intf = addInterface();
            intf->claimInterface();
            intf->addEndpoint(eps);

            stringIndex = addString("CONFIG_TINYUSB_DESC_HID_STRING", -1);

            uint8_t tmp[] = {TUD_HID_DESCRIPTOR((uint8_t)intf->ifIdx, (uint8_t)stringIndex, HID_ITF_PROTOCOL_MOUSE, _desc_hid_report.size(), (uint8_t)(0x80 | intf->endpoints.at(0)->epId), _report_len, 10)};

            if(len > 0)
                intf->setDesc(desc, len);
            else
                intf->setDesc(tmp, sizeof(tmp));

            insertDevice();
            return true;

        }

        virtual void init() final
        {
            // TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(HID_ITF_PROTOCOL_KEYBOARD) ),
            uint8_t desc[] = {TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(_report_id))};
            addHidReport(desc, sizeof(desc), 3, sizeof(hid_mouse_report_t));
        }

        virtual void setButtons(uint8_t btns) final
        {
            _report.buttons = btns;
        }
        virtual void setAxes(int8_t x, int8_t y) final
        {
            _report.x = x;
            _report.y = y;
        }
        virtual void setWheels(int8_t vert, int8_t horiz) final
        {
            _report.wheel = vert;
            _report.pan = horiz;
        }
        virtual bool sendReport(uint8_t btns, int8_t x = 0, int8_t y = 0, int8_t vert = 0, int8_t horiz = 0) final
        {
            setButtons(btns);
            setAxes(x, y);
            setWheels(horiz, vert);
            return sendReport();
        }

        virtual bool sendReport()
        {
            if (tud_hid_n_ready(_instance))
            {
                // // MOUSE: convenient helper to send mouse report if application
                // // use template layout report as defined by hid_mouse_report_t
                return tud_hid_n_mouse_report(_instance, _report_id, _report.buttons, _report.x, _report.y, _report.wheel, _report.pan);
            }
            return false;
        }
    };
} // namespace esptinyusb