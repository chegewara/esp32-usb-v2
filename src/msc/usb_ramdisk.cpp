#include "ramdisk.hpp"
// #include "esp_heap_caps.h"

// #if CONFIG_TINYUSB
// #if CONFIG_TINYUSB_MSC_ENABLED

namespace esptinyusb
{
    USBram::~USBram() {}

    void USBram::partition(uint8_t *part) // TODO change to use partition size and bool psram
    {
        _pdrv = -1;
        _partition = part;
    
        onInquiry([](uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) -> void
        {
            const char vid[] = "ESP32-S2";
            const char pid[] = "RAM disk";
            const char rev[] = "1.0";

            memcpy(vendor_id, vid, strlen(vid));
            memcpy(product_id, pid, strlen(pid));
            memcpy(product_rev, rev, strlen(rev));
        });

        onCapacity([=](uint8_t lun, uint32_t *block_count, uint16_t *block_size) -> void
        {
            *block_count = _block_count;
            *block_size = _block_size;
        });

        onRead([=](uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) -> int32_t
        {
            auto sector = _block_size;
            uint8_t *addr = &_partition[lba * sector] + offset;
            memcpy(buffer, addr, bufsize);

            return bufsize;
        });

        onWrite([=](uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) -> int32_t
        {
            auto sector = _block_size;
            auto *addr = &_partition[lba * sector] + offset;
            memcpy(addr, buffer, bufsize);

            return bufsize;
        });
    }

} // namespace esptinyusb

// #endif // CONFIG_TINYUSB_MSC_ENABLED
// #endif // CONFIG_TINYUSB
