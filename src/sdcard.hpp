#pragma once

#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "usb_msc.hpp"

// #if CONFIG_TINYUSB
// #if CONFIG_TINYUSB_MSC_ENABLED

namespace esptinyusb
{
    class SDCard2USB : public USBmsc
    {
        spi_bus_config_t bus_cfg;
        sdmmc_slot_config_t slot_config_mmc = SDMMC_SLOT_CONFIG_DEFAULT();
        sdspi_device_config_t slot_config_spi = SDSPI_DEVICE_CONFIG_DEFAULT();
        bool _is_mmc = true;

    public:
        sdmmc_card_t *_card;
        SDCard2USB();
        ~SDCard2USB();
        void initPins(uint8_t cmd, uint8_t clk, uint8_t d0, int8_t d1 = -1, int8_t d2 = -1, int8_t d3 = -1);
        bool partition(const char *path, bool mmc = true);
        void setCallbacks(USBMSCcallbacks *cb);
        void ready(bool ready);
        bool isReady();
        bool end();
        sdmmc_card_t& card();

        USBMSCcallbacks *m_private;
        uint32_t block_count = 0;
        uint32_t block_size = 512;
        bool sdcardReady = false;
    };
}
// #endif
