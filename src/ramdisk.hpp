#pragma once
#include "usb_msc.hpp"

// #if CONFIG_TINYUSB
// #if CONFIG_TINYUSB_MSC_ENABLED

namespace esptinyusb
{
    class USBram : public USBmsc
    {
    public:
        uint8_t* _partition = nullptr;
    public:
        using USBmsc::USBmsc;
        ~USBram();

        virtual void partition(uint8_t* );

    };
    
   
} // namespace esptinyusb

// #endif // CONFIG_TINYUSB_MSC_ENABLED
// #endif // CONFIG_TINYUSB
