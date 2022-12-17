#pragma once
#include "usb_msc.hpp"

#if CONFIG_TINYUSB
#if CFG_TUD_MSC

namespace esptinyusb
{
    class USBram : public USBmsc
    {
    public:
    public:
        using USBmsc::USBmsc;
        ~USBram();

        virtual void partition(uint8_t* );
    };
    
   
} // namespace esptinyusb

#endif // CFG_TUD_MSC
#endif // CONFIG_TINYUSB
