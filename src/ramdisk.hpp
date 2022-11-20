#pragma once
#include "usb_msc.hpp"
#include "esp_heap_caps.h"

// #if CONFIG_TINYUSB
// #if CONFIG_TINYUSB_MSC_ENABLED

namespace esptinyusb
{
    class USBram : public USBmsc
    {
    public:
        uint8_t* _partition = nullptr;
    public:
        USBram();
        ~USBram();

        virtual void partition(uint8_t* );

    };
    
   
} // namespace esptinyusb

// #endif // CONFIG_TINYUSB_MSC_ENABLED
// #endif // CONFIG_TINYUSB
