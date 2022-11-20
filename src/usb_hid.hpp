#pragma once
#include "private/usb_device.hpp"
#include <vector>

// #if CONFIG_TINYUSB
// #if CONFIG_TINYUSB_MSC_ENABLED

namespace esptinyusb
{

    class HIDdevice : public BaseDevice
    {
    protected:
        std::vector<uint8_t> _desc_hid_report;
        std::vector<std::pair<uint8_t, uint8_t>> _offsets; // report_id, report offset
        std::vector<std::pair <uint8_t, uint8_t>> _report_ids; // report_id, report_len
        uint8_t* _reports = nullptr;
        uint8_t _report_len = 0;
    protected:
        uint8_t _instance = 0;
        void insertDevice();

    public:
        using BaseDevice::BaseDevice;
        ~HIDdevice();

        virtual bool begin(uint8_t _eps = 1) { 
            return begin(-1, nullptr);
        };
        virtual bool begin(uint8_t eps, uint8_t* desc, size_t len = 0);

        virtual bool sendReport(uint8_t report_id) 
        {
            uint8_t _len = 0;
            uint8_t *_report = _reports;
            for (auto _id : _report_ids)
            {
                if(_id.first == report_id)
                {
                    _len = _id.second;
                }
            }
            for(auto _p : _offsets)
            {
                if(_p.first == report_id)
                {
                    _report += _p.second;
                }
            }
            
            if (tud_hid_n_ready(_instance))
            {
                return tud_hid_n_report(_instance, report_id, _report, _len);
            }
            return false;
        }
        virtual void addHidReport(uint8_t *desc, size_t len, uint8_t report_id, uint8_t report_len) final
        {
            _desc_hid_report.insert(_desc_hid_report.end(), desc, desc + len);
            auto _pair = std::make_pair(report_id, report_len);

            uint8_t _offset = 0;
            for (auto _p : _report_ids)
            {
                _offset += _p.second;
            }
            auto __pair = std::make_pair(report_id, _offset);
            _report_ids.push_back(_pair);
            _offsets.push_back(__pair);
            if(_reports == nullptr)
                _reports = (uint8_t*)calloc(1, report_len);
            else
                _reports = (uint8_t*)realloc(_reports, _offset + report_len);

            _report_len = _offset + report_len;
        }

    // private:
        virtual const uint8_t *getHidReport() final { return &_desc_hid_report[0]; }
        virtual void _onSetReport(uint8_t report_id, uint8_t const *buffer, uint16_t bufsize){} // TODO
        virtual void _onSendComplete(uint8_t const *buffer, uint16_t bufsize){} // TODO

        friend uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance);
        friend void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);
    };
} // namespace esptinyusb

// #endif // CONFIG_TINYUSB_MSC_ENABLED
// #endif // CONFIG_TINYUSB

// // Send report to host
// bool tud_hid_n_report(uint8_t instance, uint8_t report_id, void const* report, uint8_t len);

// // KEYBOARD: convenient helper to send keyboard report if application
// // use template layout report as defined by hid_keyboard_report_t
// bool tud_hid_n_keyboard_report(uint8_t instance, uint8_t report_id, uint8_t modifier, uint8_t keycode[6]);

// // MOUSE: convenient helper to send mouse report if application
// // use template layout report as defined by hid_mouse_report_t
// bool tud_hid_n_mouse_report(uint8_t instance, uint8_t report_id, uint8_t buttons, int8_t x, int8_t y, int8_t vertical, int8_t horizontal);

// // Gamepad: convenient helper to send gamepad report if application
// // use template layout report TUD_HID_REPORT_DESC_GAMEPAD
// bool tud_hid_n_gamepad_report(uint8_t instance, uint8_t report_id, int8_t x, int8_t y, int8_t z, int8_t rz, int8_t rx, int8_t ry, uint8_t hat, uint32_t buttons);
