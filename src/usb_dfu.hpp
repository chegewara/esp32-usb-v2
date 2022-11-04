#pragma once
#include "private/usb_device.hpp"

// #if CONFIG_TINYUSB
// #if CFG_TUD_DFU_RUNTIME

namespace esptinyusb
{
    /**
     * @brief DFU class lets to add interface and enable dfu updates
     * 
     * @warning currently this interface is not working with S3, not tested with S2 yet
     * 
     */
    class USBdfu : public BaseDevice
    {
    private:
    public:
        using BaseDevice::BaseDevice;
        ~USBdfu() {}

        virtual bool begin(uint8_t _eps = 1) final;
        virtual bool end() { return true; }
    };

} // namespace esptinyusb

// #endif // CFG_TUD_DFU_RUNTIME
// #endif // CONFIG_TINYUSB
