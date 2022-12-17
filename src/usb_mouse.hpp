#pragma once

#include "usb_hid.hpp"

#if CONFIG_TINYUSB
#if CFG_TUD_HID

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

        virtual bool begin(uint8_t eps, uint8_t* desc = nullptr, size_t len = 0) override;

        virtual void init(uint8_t report_id = 3) final
        {
            _report_id = report_id;
            uint8_t desc[] = {TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(_report_id))};
            addHidReport(desc, sizeof(desc), _report_id, sizeof(hid_mouse_report_t));
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
            if (tud_hid_n_ready(ifIdx))
            {
                return tud_hid_n_mouse_report(ifIdx, _report_id, _report.buttons, _report.x, _report.y, _report.wheel, _report.pan);
            }
            return false;
        }
    };
} // namespace esptinyusb

#endif
#endif
