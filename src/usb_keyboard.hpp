#pragma once

#include "usb_hid.hpp"

namespace esptinyusb
{
    class USBkeyboard : public HIDdevice
    {
    private:
        // uint8_t _report[5] = {};
        hid_keyboard_report_t _report = {};

    public:
        using HIDdevice::HIDdevice;
        ~USBkeyboard() {}

        virtual bool begin(uint8_t eps, uint8_t* desc = nullptr, size_t len = 0) override;

        virtual void init(uint8_t report_id = 2) final
        {
            _report_id = report_id;
            uint8_t desc[] = {TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(_report_id))};
            addHidReport(desc, sizeof(desc), _report_id, sizeof(hid_keyboard_report_t));
        }

        virtual uint8_t keyCode(const char c)
        {
            uint8_t const conv_table[128][2] =  { HID_ASCII_TO_KEYCODE };
            return conv_table[c][1];
        }

        virtual bool sendKey(const char c)
        {
            uint8_t const conv_table[128][2] =  { HID_ASCII_TO_KEYCODE };
            if ( conv_table[c][0] ) _report.modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
            _report.keycode[0] = conv_table[c][1];
            return sendReport();
        }

        virtual bool sendKey(uint8_t key1 = 0) final
        {
            _report.keycode[0] = key1;
            _report.keycode[1] = 0;
            _report.keycode[2] = 0;
            _report.keycode[3] = 0;
            _report.keycode[4] = 0;
            _report.keycode[5] = 0;
            return sendReport();
        }

        virtual bool sendKey(uint8_t mod, uint8_t key1, uint8_t key2 = 0, uint8_t key3 = 0, uint8_t key4 = 0, uint8_t key5 = 0, uint8_t key6 = 0) final
        {
            _report.modifier = mod;
            _report.keycode[0] = key1;
            _report.keycode[1] = key2;
            _report.keycode[2] = key3;
            _report.keycode[3] = key4;
            _report.keycode[4] = key5;
            _report.keycode[5] = key6;
            return sendReport();
        }

        virtual bool sendReport()
        {
            if (tud_hid_n_ready(ifIdx))
            {
                return tud_hid_n_keyboard_report(ifIdx, _report_id, _report.modifier, _report.keycode);
            }
            return false;
        }
    };
} // namespace esptinyusb
