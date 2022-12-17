#pragma once
#include <memory>
#include <functional>
#include "private/usb_device.hpp"

#if CONFIG_TINYUSB
#if CFG_TUD_MSC

namespace esptinyusb
{
    class USBmsc;
    // class USBMSCcallbacks
    // {
    // protected:
    //     uint8_t _lun = 0;

    // public:
    //     USBMSCcallbacks() {}
    //     ~USBMSCcallbacks() {}

    //     virtual void onInquiry(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]) {}
    //     virtual bool onReady(uint8_t lun) { return true; }
    //     virtual void onCapacity(uint8_t lun, uint32_t *block_count, uint16_t *block_size) {}
    //     virtual bool onStop(uint8_t lun, uint8_t power_condition, bool start, bool load_eject) { return true; }
    //     virtual int32_t onRead(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) { return -1; }
    //     virtual int32_t onWrite(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) { return -1; }
    // };

    class USBmsc : public BaseDevice
    {
        uint8_t _lun = 0;
    protected:
        // typedef void (*on_inquiry_t)(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]);
        // typedef bool (*on_ready_t)(uint8_t lun);
        // typedef void (*on_capacity_t)(uint8_t lun, uint32_t *block_count, uint16_t *block_size);
        // typedef bool (*on_stop_t)(uint8_t lun, uint8_t power_condition, bool start, bool load_eject);
        // typedef int32_t (*on_read_t)(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
        // typedef int32_t (*on_write_t)(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
        int8_t _pdrv = 0;

        std::function<void (uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])> _inq_cb;
        std::function<bool (uint8_t lun)> _ready_cb = [](uint8_t lun) { return true; };
        std::function<void (uint8_t lun, uint32_t *block_count, uint16_t *block_size)> _capacity_cb;
        std::function<bool (uint8_t lun, uint8_t power_condition, bool start, bool load_eject)> _stop_cb = [](uint8_t lun, uint8_t power_condition, bool start, bool load_eject) { return true; };
        std::function<int32_t (uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)> _read_cb;
        std::function<int32_t (uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)> _write_cb;

    public:
        // USBMSCcallbacks *_callbacks = nullptr;
        static uint8_t _luns;
        uint32_t _block_count = 0;
        uint32_t _block_size = 0;
        static uint8_t _pdrvs;
        uint8_t* _partition = nullptr;

    public:
        USBmsc();
        ~USBmsc();

        virtual bool begin(uint8_t _eps = 1);
        virtual bool end();
        virtual int pdrv() final { return _pdrv; }
        // virtual void callbacks(USBMSCcallbacks *cb) final { delete(_callbacks); _callbacks = cb; };
        virtual uint8_t luns() final { return _lun; }
        virtual void luns(uint8_t count) final { _luns = count; }
        virtual void setCapacity(size_t block_count, size_t block_size = 4096) final { _block_count = block_count; _block_size = block_size; }

        virtual void onInquiry(std::function<void (uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])> cb) final {
            _inq_cb = cb;
        }
        virtual void onReady(std::function<bool (uint8_t lun)> cb) final {
            _ready_cb = cb;
        }
        virtual void onCapacity(std::function<void (uint8_t lun, uint32_t *block_count, uint16_t *block_size)> cb) final {
            _capacity_cb = cb;
        }
        virtual void onStop(std::function<bool (uint8_t lun, uint8_t power_condition, bool start, bool load_eject)> cb) final {
            _stop_cb = cb;
        }
        virtual void onRead(std::function<int32_t (uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)> cb) final {
            _read_cb = cb;
        }
        virtual void onWrite(std::function<int32_t (uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)> cb) final {
            _write_cb = cb;
        }

    private:
        virtual void _onInquiry(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]);
        virtual bool _onReady(uint8_t lun);
        virtual void _onCapacity(uint8_t lun, uint32_t *block_count, uint16_t *block_size);
        virtual bool _onStop(uint8_t lun, uint8_t power_condition, bool start, bool load_eject);
        virtual int32_t _onRead(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
        virtual int32_t _onWrite(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);

        friend void ::tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4]);
        friend bool ::tud_msc_test_unit_ready_cb(uint8_t lun);
        friend void ::tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size);
        friend bool ::tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject);
        friend int32_t ::tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize);
        friend int32_t ::tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize);
        friend int32_t ::tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize);
    };
}

#endif // CFG_TUD_MSC
#endif // CONFIG_TINYUSB
