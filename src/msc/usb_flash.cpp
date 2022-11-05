#include "flashdisk.hpp"
#include "esp_heap_caps.h"
#include <deque>

#if CONFIG_TINYUSB_ENABLED
#if CONFIG_TINYUSB_MSC_ENABLED

namespace esptinyusb
{
    class Callbacks : public USBMSCcallbacks
    {
#ifdef CONFIG_SPIRAM
        struct write_s
        {
            uint32_t lba;
            uint32_t offset;
            uint32_t bufsize;
            void *buffer;
        };
        std::deque<write_s *> _write_data;
        bool write_cache(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
        {
            if(_write_data.size() > 5) return false;
            auto _buf = heap_caps_calloc(1, bufsize, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
            if (_buf)
            {
                auto wr = new write_s();
                wr->bufsize = bufsize;
                wr->buffer = _buf;
                wr->lba = lba;
                wr->offset = offset;
                memcpy(_buf, buffer, bufsize);
                _write_data.push_back(wr);
                return true;
            }
            return false;
        }

        void read_cache()
        {
            for (size_t i = 0; i < 5; i++)
            {
                if (!_write_data.empty())
                {
                    // printf("read_cache: %d\n", _write_data.size());
                    auto wr = _write_data.front();
                    disk_write(s_pdrv, (BYTE *)wr->buffer, wr->lba, 1);
                    _write_data.pop_front();
                    free(wr->buffer);
                    delete (wr);
                }
            }

            // {
            //     if (disk_ioctl(s_pdrv, CTRL_SYNC, NULL) != RES_OK)
            //     {
            //         printf("failed to sync\n");
            //         // return false;
            //     }
            // }
        }
#endif

        USBflash *_device = nullptr;
        int s_pdrv = 0;
        bool eject = true;
#if CONFIG_WL_SECTOR_SIZE > CONFIG_TINYUSB_MSC_BUFSIZE
        uint8_t _buf[CONFIG_WL_SECTOR_SIZE] = {};
#endif

    public:
        Callbacks(USBflash *msc, int pdrv = 0, uint8_t lun = 0)
        {
            _lun = lun;
            s_pdrv = pdrv;
            _device = msc;
        }

        bool onStop(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
        {

            (void)lun;
            (void)power_condition;
            while (!_write_data.empty())
            {
                // printf("read_cache: %d\n", _write_data.size());
                read_cache();
            }

            if (load_eject)
            {
                if (!start)
                {
                    // Eject but first flush.
                    if (disk_ioctl(s_pdrv, CTRL_SYNC, NULL) != RES_OK)
                    {
                        eject = false;
                        return false;
                    }
                    else
                    {
                        eject = true;
                    }
                }
                else
                {
                    // We can only load if it hasn't been ejected.
                    return !eject;
                }
            }
            else
            {
                if (!start)
                {
                    // Stop the unit but don't eject.
                    if (disk_ioctl(s_pdrv, CTRL_SYNC, NULL) != RES_OK)
                    {
                        return false;
                    }
                }
            }

            return true;
        }
        bool onReady(uint8_t lun)
        {
            // printf("ready flash: %d\n", lun);
            if (lun != _lun)
                return false;
#ifdef CONFIG_SPIRAM
            read_cache();
#endif
            return _device->sdcardReady;
        }

        void onInquiry(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
        {
            // printf("inquiry\n");
            if (lun != _lun)
                return;
            const char vid[] = "ESP32-S3";
            const char pid[16] = "Flash disk";
            const char rev[4] = "1.0";

            memcpy(vendor_id, vid, 8);
            memcpy(product_id, pid, 16);
            memcpy(product_rev, rev, 4);
        }

        void onCapacity(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
        {
            // printf("capacity\n");
            if (lun != _lun)
                return;
            disk_ioctl(s_pdrv, GET_SECTOR_COUNT, block_count);
            disk_ioctl(s_pdrv, GET_SECTOR_SIZE, block_size);
            _device->_block_count = *block_count;
            _device->_block_size = *block_size;
        }

        int32_t onRead(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
        {
            if (lun != _lun)
                return -1;
#if CONFIG_WL_SECTOR_SIZE > CONFIG_TINYUSB_MSC_BUFSIZE
            disk_read(s_pdrv, _buf, lba, 1);
            memcpy(buffer, &_buf[offset], bufsize);
#else
            disk_read(s_pdrv, (BYTE *)buffer, lba, 1);
#endif

            return bufsize;
        }

        int32_t onWrite(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
        {

            if (lun != _lun)
                return -1;
#if CONFIG_WL_SECTOR_SIZE > CONFIG_TINYUSB_MSC_BUFSIZE
            disk_read(s_pdrv, _buf, lba, 1);
            memcpy(&_buf[offset], buffer, bufsize);
            disk_write(s_pdrv, _buf, lba, 1);
#else
            if (!write_cache(lba, offset, buffer, bufsize))
            {
                printf("failed to cache\n");
                disk_write(s_pdrv, (BYTE *)buffer, lba, 1);
                // return -1;
            }
#endif

            return bufsize;
        }
    };

    USBflash::USBflash()
    {
        static int pdrv = 0;
        callbacks(new Callbacks(this, pdrv));
        pdrv++;
    }
    USBflash::~USBflash()
    {
    }

    void USBflash::partition(const char *path, const char *label)
    {
        esp_vfs_fat_mount_config_t mount_config =
            {
                .format_if_mount_failed = true,
                .max_files = 5,
                .allocation_unit_size = 2 * 4096,
                // .disk_status_check_enable = false,
            };

        esp_err_t err = esp_vfs_fat_spiflash_mount(path, label, &mount_config, &wl_handle);
        if (!err)
        {
            setCapacity(wl_size(wl_handle) / wl_sector_size(wl_handle), wl_sector_size(wl_handle));
            sdcardReady = true;
        }

        // return err == ESP_OK;
    }

    bool USBflash::end()
    {
        // esp_vfs_fat_spiflash_unmount_rw_wl(const char* base_path, wl_handle_t wl_handle);
        return true;
    }

} // namespace esptinyusb

// Invoked when Read10 command is complete
TU_ATTR_WEAK void tud_msc_read10_complete_cb(uint8_t lun)
{
    // printf("read completed\n");
}
// Invoke when Write10 command is complete, can be used to flush flash caching
TU_ATTR_WEAK void tud_msc_write10_complete_cb(uint8_t lun)
{
    // printf("write completed\n");
}

#endif // CONFIG_TINYUSB_MSC_ENABLED
#endif // CONFIG_TINYUSB_ENABLED
