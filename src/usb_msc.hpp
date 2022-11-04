#pragma once
#include <memory>
#include <functional>
#include "usb_device.hpp"

namespace esptinyusb
{
    class USBMSC;
    class USBMSCcallbacks
    {
    protected:
        uint8_t _lun = 0;

    public:
        USBMSCcallbacks() {}
        ~USBMSCcallbacks() {}

        virtual void onInquiry(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {}
        virtual bool onReady(uint8_t lun) { return true; }
        virtual void onCapacity(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {}
        virtual bool onStop(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) { return true; }
        virtual int32_t onRead(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) { return -1; }
        virtual int32_t onWrite(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) { return -1; }
    };

    class USBMSC : public BaseDevice
    {
        typedef void (*on_inquiry_t)(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]);
        typedef bool (*on_ready_t)(uint8_t lun);
        typedef void (*on_capacity_t)(uint8_t lun, uint32_t *block_count, uint16_t *block_size);
        typedef bool (*on_stop_t)(uint8_t lun, uint8_t power_condition, bool start, bool load_eject);
        typedef int32_t (*on_read_t)(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
        typedef int32_t (*on_write_t)(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);

    public:
        USBMSCcallbacks *_callbacks = nullptr;
        uint8_t _luns = 0;
        uint32_t _block_count = 0;
        uint32_t _block_size = 0;

    public:
        USBMSC();
        ~USBMSC();

        virtual bool begin(uint8_t _eps = 1);
        virtual bool end();
        virtual void callbacks(USBMSCcallbacks *cb) final { _callbacks = cb; };
        virtual uint8_t luns() final { return _luns; }
        virtual void luns(uint8_t count) final { _luns = count; }
        virtual void setCapacity(size_t block_count, size_t block_size = 4096) final { _block_count = block_count; _block_size = block_size; }

        virtual void _onInquiry(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]);
        virtual bool _onReady(uint8_t lun);
        virtual void _onCapacity(uint8_t lun, uint32_t *block_count, uint16_t *block_size);
        virtual bool _onStop(uint8_t lun, uint8_t power_condition, bool start, bool load_eject);
        virtual int32_t _onRead(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
        virtual int32_t _onWrite(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
    };
}
