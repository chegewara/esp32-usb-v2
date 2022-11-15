#include "../sdcard.hpp"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "../usb_msc.hpp"

namespace esptinyusb
{
#define MOUNT_POINT "/sdcard"
#define ESSL_CMD53_END_ADDR    0x1f800




    class SDCallbacks : public USBMSCcallbacks
    {
        SDCard2USB *m_parent;

    public:
        SDCallbacks(SDCard2USB *ram) { m_parent = ram; }
        ~SDCallbacks() {}
        void onInquiry(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
        {
            if (m_parent->m_private)
            {
                m_parent->m_private->onInquiry(lun, vendor_id, product_id, product_rev);
            }
            else
            {

                const char vid[] = "";
                const char pid[] = "";
                const char rev[] = "1.0";
                memcpy(product_id, vid, strlen(vid));
                if(m_parent->isReady())
                {
                    
                    memcpy(vendor_id, m_parent->card().cid.name, 8);
                }
                memcpy(product_id, pid, strlen(pid));
                memcpy(product_rev, rev, strlen(rev));
            }
        }
        bool onReady(uint8_t lun)
        {
            if (m_parent->m_private)
            {
                return m_parent->m_private->onReady(lun);
            }
            else
            {
                return m_parent->isReady(); // RAM disk is always ready
            }
        }
        void onCapacity(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
        {
            (void)lun;
            
            *block_count = (uint64_t)m_parent->card().csd.capacity; // * m_parent->card().csd.sector_size;
            *block_size = m_parent->card().csd.sector_size;
            printf("ram disk block count: %d, block size: %d (%lld)\n", *block_count, *block_size, (uint64_t)m_parent->card().csd.capacity);
        }
        bool onStop(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
        {
            (void)lun;
            (void)power_condition;

            if (load_eject)
            {
                if (start)
                {
                    // load disk storage
                }
                else
                {
                    // unload disk storage
                }
            }

            return true;
        }
        int32_t onRead(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
        {
            (void)lun;
            // printf("read: lun => %d, lba => %d, offset => %d, size => %d\n", lun, lba, offset, bufsize);
            auto _bp = buffer;
            auto _buf = heap_caps_calloc(1, bufsize, MALLOC_CAP_DMA);
            if(_buf){
                _bp = _buf;
            }
            size_t sector_count = bufsize / m_parent->card().csd.sector_size;
            auto err = sdmmc_read_sectors(&m_parent->card(), _bp, lba, sector_count);
            if(_buf)
                memcpy(buffer, _buf, bufsize);
            heap_caps_free(_buf);
            if(err) return 0;
            return bufsize;
        }
        int32_t onWrite(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
        {
            (void)lun;
            // printf("write: lun => %d, lba => %d, offset => %d, size => %d\n", lun, lba, offset, bufsize);
            auto _bp = buffer;
            auto _buf = heap_caps_calloc(1, bufsize, MALLOC_CAP_DMA);
            if(_buf){
                memcpy(_buf, buffer, bufsize);
                _bp = _buf;
            }
            auto sector_count = bufsize / m_parent->card().csd.sector_size;
            auto err = sdmmc_write_sectors(&m_parent->card(), _bp, lba, sector_count);
            heap_caps_free(_buf);
            if(err) return 0;
            return bufsize;
        }
    };
IRAM_ATTR SDCallbacks* cb = nullptr;
    SDCard2USB::SDCard2USB()
    {
        static int pdrv = 0;
        cb = new SDCallbacks(this);
        callbacks(cb);
    }

    SDCard2USB::~SDCard2USB()
    {

    }

    void SDCard2USB::initPins(uint8_t cmd, uint8_t clk, uint8_t d0, int8_t d1, int8_t d2, int8_t d3)
    {
        slot_config_mmc.width = 1;
        slot_config_mmc.cmd = (gpio_num_t)cmd;
        slot_config_mmc.clk = (gpio_num_t)clk;
        slot_config_mmc.d0 = (gpio_num_t)d0;

        if(d1 >= 0 && d2 >= 0 && d3 >= 0)
        {
            slot_config_mmc.width = 4;
            slot_config_mmc.d1 = (gpio_num_t)d1;
            slot_config_mmc.d2 = (gpio_num_t)d2;
            slot_config_mmc.d3 = (gpio_num_t)d3;
        }
        slot_config_mmc.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

        bus_cfg.mosi_io_num = cmd;
        bus_cfg.miso_io_num = d0;
        bus_cfg.sclk_io_num = clk;
        bus_cfg.quadwp_io_num = d1;
        bus_cfg.quadhd_io_num = d2;
        bus_cfg.max_transfer_sz = 4000;
        if(d1 >= 0)
            slot_config_spi.gpio_cs = (gpio_num_t)d1;
        else if(d3 >= 0)
            slot_config_spi.gpio_cs = (gpio_num_t)d3;
    }

    bool SDCard2USB::partition(const char *path, bool mmc)
    {
        auto ret = ESP_OK;
        esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = true,
            .max_files = 5,
            .allocation_unit_size = 16 * 1024
        };
        _is_mmc = mmc;

        if(mmc)
        {
            sdmmc_host_t host = SDMMC_HOST_DEFAULT();
            host.max_freq_khz = SDMMC_FREQ_52M;
            host.flags &= ~SDMMC_HOST_FLAG_DDR;
            ret = esp_vfs_fat_sdmmc_mount(path, &host, &slot_config_mmc, &mount_config, &_card);
        }
        else
        {
            sdmmc_host_t host = SDSPI_HOST_DEFAULT();
            host.max_freq_khz = SDMMC_FREQ_DEFAULT;
            spi_bus_initialize((spi_host_device_t)host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
            slot_config_spi.host_id = (spi_host_device_t)host.slot;
            ret = esp_vfs_fat_sdspi_mount(path, &host, &slot_config_spi, &mount_config, &_card);
        }

        if(ret) return false;
        sdcardReady = true;
        return true;
    }

    sdmmc_card_t& SDCard2USB::card()
    {
        return *_card;
    }

    void SDCard2USB::setCallbacks(USBMSCcallbacks *cb)
    {
        m_private = cb;
    }

    void SDCard2USB::ready(bool ready)
    {
    }

    bool SDCard2USB::isReady()
    {
        return sdcardReady;
    }


    bool SDCard2USB::end()
    {
        if(_is_mmc)
            esp_vfs_fat_sdmmc_unmount();
        else
            esp_vfs_fat_sdcard_unmount("/sdcard", _card);
        return true;
    }
}
