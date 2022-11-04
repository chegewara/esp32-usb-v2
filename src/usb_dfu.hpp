#pragma once
#include "usb_device.hpp"

#if CFG_TUD_DFU_RUNTIME

namespace esptinyusb
{

    class USBdfu : public BaseDevice
    {
    private:
    public:
        USBdfu() {}
        ~USBdfu() {}

        virtual bool begin(uint8_t _eps = 1) final;
        virtual bool end() { return true; }
    };

} // namespace esptinyusb

#endif
