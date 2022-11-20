#pragma once
#include "esp_vfs_fat.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"

#include "usb_msc.hpp"

// #if CONFIG_TINYUSB
// #if CONFIG_TINYUSB_MSC_ENABLED

namespace esptinyusb
{
    class USBflash : public USBmsc
    {
    public:
        const char *base_path = "/fatfs";
        const char *partition_label = NULL;
        wl_handle_t wl_handle = -1;
        bool sdcardReady = false;

    public:
        using USBmsc::USBmsc;
        ~USBflash();
        virtual bool end();

        virtual bool partition(const char *path, const char *label);
    };

} // namespace esptinyusb

// #endif // CONFIG_TINYUSB_MSC_ENABLED
// #endif // CONFIG_TINYUSB
