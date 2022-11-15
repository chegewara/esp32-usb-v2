#pragma once

// #include "usb/usb_types_stack.h"
// #include "usb/usb_types_ch9.h"

// #if CONFIG_TINYUSB

namespace esptinyusb
{

    class USBEndpoint
    {
    // private:
        //
    public:
        uint8_t epId = 0;
        USBEndpoint() = delete;
        USBEndpoint(uint8_t id) : epId(id){}
        ~USBEndpoint(){}

    public:
        virtual bool claimEndpoint(){ return true; }
        virtual bool releaseEndpoint(){ return true; }
    };


}

// #endif // CONFIG_TINYUSB
