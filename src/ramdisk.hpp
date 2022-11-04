#pragma once
#include "usb_msc.hpp"
#include "esp_heap_caps.h"

namespace esptinyusb
{
    class USBRam : public USBMSC
    {
    public:
        uint8_t* _partition = nullptr;
    public:
        USBRam();
        ~USBRam();

        virtual void partition(uint8_t* );

    };
    
   
} // namespace esptinyusb

