
#include "usb_dfu.hpp"

#if CONFIG_TINYUSB_ENABLED
#if CFG_TUD_DFU_RUNTIME

namespace esptinyusb
{

    bool USBdfu::begin(uint8_t eps)
    {
        auto intf = addInterface();
        intf->claimInterface();

        stringIndex = addString(CONFIG_TINYUSB_DESC_DFU_RT_STRING, -1);

        // Interface number, string index, attributes, detach timeout, transfer size
        uint8_t tmp[] = {TUD_DFU_RT_DESCRIPTOR((uint8_t)intf->ifIdx, (uint8_t)stringIndex, 0x0f, 1000, 1024)};
        intf->setDesc(tmp, sizeof(tmp));

        return true;
    }

} // namespace esptinyusb

/**
 * enter dfu bootload mode
 */
void tud_dfu_rt_reboot_to_dfu(void)
{
    esptinyusb::persistentReset(RESTART_BOOTLOADER_DFU);
}

#endif // CFG_TUD_DFU_RUNTIME
#endif // CONFIG_TINYUSB_ENABLED
