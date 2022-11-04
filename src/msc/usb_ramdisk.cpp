#include "ramdisk.hpp"

namespace esptinyusb
{
    class Callbacks : public USBMSCcallbacks
    {
        USBRam *_device = nullptr;

    public:
        Callbacks(USBRam *msc, uint8_t lun = 0)
        {
            _lun = lun;
            _device = msc;
        }

        void onInquiry(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
        {
            const char vid[] = "ESP32-S2";
            const char pid[] = "RAM disk";
            const char rev[] = "1.0";

            memcpy(vendor_id, vid, strlen(vid));
            memcpy(product_id, pid, strlen(pid));
            memcpy(product_rev, rev, strlen(rev));
        }

        void onCapacity(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
        {
            *block_count = _device->_block_count;
            *block_size = _device->_block_size;
        }

        int32_t onRead(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
        {
            auto sector = _device->_block_size;
            uint8_t *addr = &_device->_partition[lba * sector] + offset;
            memcpy(buffer, addr, bufsize);

            return bufsize;
        }

        int32_t onWrite(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
        {
            auto sector = _device->_block_size;
            uint8_t *addr = &_device->_partition[lba * sector] + offset;
            memcpy(addr, buffer, bufsize);

            return bufsize;
        }
    };

    USBRam::USBRam()
    {
        callbacks(new Callbacks(this, 1));
    }
    USBRam::~USBRam()
    {
    }

    void USBRam::partition(uint8_t *part)
    {
        _partition = part;
    }

} // namespace esptinyusb
