#include <stdio.h>
#include <memory>
#include "hal/usb_hal.h"
#include "soc/usb_periph.h"
#include "driver/periph_ctrl.h"
#include "driver/gpio.h"
#include "soc/rtc_cntl_reg.h"
#include "sdkconfig.h"

#include "soc/usb_struct.h"
#include "soc/usb_reg.h"
#include "soc/usb_wrap_reg.h"
#include "soc/usb_wrap_struct.h"

// #if CONFIG_TINYUSB
#include "soc/soc.h"
#include "soc/efuse_reg.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/usb_struct.h"
#include "soc/usb_reg.h"
#include "soc/usb_wrap_reg.h"
#include "soc/usb_wrap_struct.h"
#include "soc/usb_periph.h"
#include "soc/periph_defs.h"
#include "soc/timer_group_struct.h"
#include "soc/system_reg.h"

#include "hal/usb_hal.h"
#include "hal/gpio_ll.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/periph_ctrl.h"

#include "esp_efuse.h"
#include "esp_efuse_table.h"
#include "esp_rom_gpio.h"

#if CONFIG_IDF_TARGET_ESP32S2
#include "esp32s2/rom/usb/usb_persist.h"
#include "esp32s2/rom/usb/usb_dc.h"
#include "esp32s2/rom/usb/chip_usb_dw_wrapper.h"
#elif CONFIG_IDF_TARGET_ESP32S3
#include "hal/usb_serial_jtag_ll.h"
#include "esp32s3/rom/usb/usb_persist.h"
#include "esp32s3/rom/usb/usb_dc.h"
#include "esp32s3/rom/usb/chip_usb_dw_wrapper.h"
#endif
#include "esp_log.h"

#include "../private/usb_device.hpp"

void printf_buffer(const uint8_t *buffer, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
    ESP_LOG_BUFFER_HEX("", buffer, len);
}

namespace esptinyusb
{
    static void esptinyusbtask(void *p)
    {
        (void)p;
        printf("USB tud_task created\n");
        while (1)
        {
            // tinyusb device task
            tud_task();
        }
    }
    int usb_persist_mode = RESTART_NO_PERSIST;

    static void IRAM_ATTR usb_persist_shutdown_handler(void)
    {
        if(usb_persist_mode != RESTART_NO_PERSIST){
            int usb_persist_enabled = 0;
            if (usb_persist_enabled) {
                usb_dc_prepare_persist();
            }
            if (usb_persist_mode == RESTART_BOOTLOADER) {
                //USB CDC Download
                if (usb_persist_enabled) {
                    chip_usb_set_persist_flags(USBDC_PERSIST_ENA);
    #if CONFIG_IDF_TARGET_ESP32S2
                } else {
                    periph_module_reset(PERIPH_USB_MODULE);
                    periph_module_enable(PERIPH_USB_MODULE);
    #endif
                }
                // REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT); /// @fixme for some reason this is causing S3 to hang
            } else if (usb_persist_mode == RESTART_BOOTLOADER_DFU) {
                //DFU Download
    #if CONFIG_IDF_TARGET_ESP32S2
                // Reset USB Core
                USB0.grstctl |= USB_CSFTRST;
                while ((USB0.grstctl & USB_CSFTRST) == USB_CSFTRST){}
    #endif
                chip_usb_set_persist_flags(USBDC_BOOT_DFU);
                REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
            } else if (usb_persist_enabled) {
                //USB Persist reboot
                chip_usb_set_persist_flags(USBDC_PERSIST_ENA);
            }
        }
    }

#if CONFIG_IDF_TARGET_ESP32S3

static void hw_cdc_reset_handler(void *arg) {
    portBASE_TYPE xTaskWoken = 0;
    uint32_t usbjtag_intr_status = usb_serial_jtag_ll_get_intsts_mask();
    usb_serial_jtag_ll_clr_intsts_mask(usbjtag_intr_status);
    
    if (usbjtag_intr_status & USB_SERIAL_JTAG_INTR_BUS_RESET) {
        xSemaphoreGiveFromISR((xSemaphoreHandle)arg, &xTaskWoken);
    }

    if (xTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

static void usb_switch_to_cdc_jtag()
{
    // Disable USB-OTG
    periph_module_reset(PERIPH_USB_MODULE);
    //periph_module_enable(PERIPH_USB_MODULE);
    periph_module_disable(PERIPH_USB_MODULE);

    // Switch to hardware CDC+JTAG
    CLEAR_PERI_REG_MASK(RTC_CNTL_USB_CONF_REG, (RTC_CNTL_SW_HW_USB_PHY_SEL|RTC_CNTL_SW_USB_PHY_SEL|RTC_CNTL_USB_PAD_ENABLE));

    // Do not use external PHY
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_PHY_SEL);

    // Release GPIO pins from  CDC+JTAG
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_USB_PAD_ENABLE);

    // Force the host to re-enumerate (BUS_RESET)
    gpio_set_direction((gpio_num_t)USBPHY_DM_NUM, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_direction((gpio_num_t)USBPHY_DP_NUM, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level((gpio_num_t)USBPHY_DM_NUM, 0);
    gpio_set_level((gpio_num_t)USBPHY_DP_NUM, 0);

    // Initialize CDC+JTAG ISR to listen for BUS_RESET
    usb_serial_jtag_ll_disable_intr_mask(USB_SERIAL_JTAG_LL_INTR_MASK);
    usb_serial_jtag_ll_clr_intsts_mask(USB_SERIAL_JTAG_LL_INTR_MASK);
    usb_serial_jtag_ll_ena_intr_mask(USB_SERIAL_JTAG_INTR_BUS_RESET);
    intr_handle_t intr_handle = NULL;
    xSemaphoreHandle reset_sem = xSemaphoreCreateBinary();
    if(reset_sem){
        if(esp_intr_alloc(ETS_USB_SERIAL_JTAG_INTR_SOURCE, 0, hw_cdc_reset_handler, reset_sem, &intr_handle) != ESP_OK){
            vSemaphoreDelete(reset_sem);
            reset_sem = NULL;
        }
    }

    // Connect GPIOs to integrated CDC+JTAG
    SET_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_USB_PAD_ENABLE);

    // Wait for BUS_RESET to give us back the semaphore
    if(reset_sem){
        if(xSemaphoreTake(reset_sem, 1000 / portTICK_PERIOD_MS) != pdPASS){
        }
        usb_serial_jtag_ll_disable_intr_mask(USB_SERIAL_JTAG_LL_INTR_MASK);
        esp_intr_free(intr_handle);
        vSemaphoreDelete(reset_sem);
    }
}
#endif

    void persistentReset(restart_type_t _usb_persist_mode)
    {
    if (_usb_persist_mode < RESTART_TYPE_MAX && esp_register_shutdown_handler(usb_persist_shutdown_handler) == ESP_OK) {
        usb_persist_mode = _usb_persist_mode;

    #if CONFIG_IDF_TARGET_ESP32S3
            if (_usb_persist_mode == RESTART_BOOTLOADER) {
                usb_switch_to_cdc_jtag();
            }
    #endif
            esp_restart();
        }
    }
    // esp_unregister_shutdown_handler(esp_usb_console_before_restart);

    std::shared_ptr<USBdevice> USBdevice::_instance = nullptr;

    std::shared_ptr<USBdevice> USBdevice::getInstance()
    {
        if (_instance == nullptr)
        {
            _instance = std::make_shared<USBdevice>();
        }
        return _instance;
    }

    void USBdevice::_init_hardware()
    {
        if (!isEnabled)
        {
            isEnabled = true;
            setDefaultDescriptorStrings();

            bool usb_did_persist = (USB_WRAP.date.val == USBDC_PERSIST_ENA);

            if (usb_did_persist)
            {
                // Enable USB/IO_MUX peripheral reset, if coming from persistent reboot
                REG_CLR_BIT(RTC_CNTL_USB_CONF_REG, RTC_CNTL_IO_MUX_RESET_DISABLE);
                REG_CLR_BIT(RTC_CNTL_USB_CONF_REG, RTC_CNTL_USB_RESET_DISABLE);
            }

            if (1)
            {
                // Reset USB module
                periph_module_reset((periph_module_t)PERIPH_USB_MODULE);
                periph_module_enable((periph_module_t)PERIPH_USB_MODULE);
            }

            // Hal init
            usb_hal_context_t hal = {
                .use_external_phy = false};
            usb_hal_init(&hal);
            /* usb_periph_iopins currently configures USB_OTG as USB Device.
             * Introduce additional parameters in usb_hal_context_t when adding support
             * for USB Host.
             */
            for (const usb_iopin_dsc_t *iopin = usb_periph_iopins; iopin->pin != -1; ++iopin)
            {
                if ((iopin->ext_phy_only == 0))
                {
                    gpio_pad_select_gpio(iopin->pin);
                    if (iopin->is_output)
                    {
                        gpio_matrix_out(iopin->pin, iopin->func, false, false);
                    }
                    else
                    {
                        gpio_matrix_in(iopin->pin, iopin->func, false);
                        gpio_pad_input_enable(iopin->pin);
                    }
                    gpio_pad_unhold(iopin->pin);
                }
            }
            gpio_set_drive_capability((gpio_num_t)USBPHY_DP_NUM, GPIO_DRIVE_CAP_3);
            gpio_set_drive_capability((gpio_num_t)USBPHY_DM_NUM, GPIO_DRIVE_CAP_3);
            // esp_register_shutdown_handler(usb_persist_shutdown_handler);
        }

        // if (!myEvents)
        // {
        //     myEvents = new EventLoop();
        //     myEvents->init("usb_device");
        // }
    }

    USBdevice::~USBdevice()
    {
        printf("USBdevice destructor\n");
    }

    bool USBdevice::_init()
    {
        _init_hardware();

        if (tusb_inited())
        {
            return true;
        }

        if (!tusb_init())
        {
            printf("failed to init\n");
            return false;
        }

        if (usbTaskHandle != nullptr)
        {
            return true;
        }

        return xTaskCreate(esptinyusbtask, "espUSB", 4 * 1024, NULL, 24, &usbTaskHandle) == pdTRUE;
    }

    uint8_t *USBdevice::getDeviceDescriptor()
    {
        return (uint8_t *)&_desc;
    }

    uint8_t *USBdevice::getConfigDescriptor()
    {
        int CONFIG_TOTAL_LEN = TUD_CONFIG_DESC_LEN;
        size_t total_len = CONFIG_TOTAL_LEN;

        if (buffer)
            free(buffer);

        uint8_t _itf_count = 0;
        for (auto _v : _interfaces)
        {
            auto len = _v->getLength();
            total_len += len;
            _itf_count++; // TODO how to solve problem with multiple interfaces???
        }

        buffer = (uint8_t *)calloc(1, total_len);
        int offset = TUD_CONFIG_DESC_LEN;
        for (auto _v : _interfaces)
        {
            auto ptr = _v->getDesc();
            auto len = _v->getLength();
            memcpy(buffer + offset, ptr, len);
            offset += len;
        }

        // interface count, string index, total length, attribute, power in mA
        uint8_t dcd[TUD_CONFIG_DESC_LEN] = {TUD_CONFIG_DESCRIPTOR(1, _itf_count, 0, total_len, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500)};
        memcpy(buffer, dcd, TUD_CONFIG_DESC_LEN);
printf_buffer(buffer, total_len);
        return buffer;
    }

    const uint16_t *USBdevice::getStringDescriptor(uint8_t index, uint16_t langid)
    {
        static uint16_t _desc_str[32] = {};
        bzero(_desc_str, 32);
        const char *str = nullptr;

        for (auto _str : stringsDesc)
        {
            if (_str->getId() == index)
            {
                str = _str->getDescriptor();
                break;
            }
        }

        uint8_t chr_count = 0;
        if (index == 0)
        {
            memcpy(&_desc_str[1], str, 2);
            chr_count = 1;
        }
        else
        {
            if (str == nullptr)
                return NULL;
            // Cap at max char
            chr_count = strlen(str);
            if (chr_count > 31)
            {
                chr_count = 31;
            }
            for (uint8_t i = 0; i < chr_count; i++)
            {
                _desc_str[1 + i] = str[i];
            }
        }

        // first byte is len, second byte is string type
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

        return _desc_str;
    }

    void USBdevice::setDefaultDescriptorStrings()
    {
        const char langID[3] = {0x09, 0x04};
        auto str = new StringDescriptor(langID, 0);
        stringsDesc.push_back(str);

        mfgStringId(addString(CONFIG_TINYUSB_DESC_MANUFACTURER_STRING));
        productStringId(addString(CONFIG_TINYUSB_DESC_PRODUCT_STRING));
        serialStringId(addString(CONFIG_TINYUSB_DESC_SERIAL_STRING));
    }

    uint8_t USBdevice::addString(const char *pointer, int8_t id)
    {
        assert(id != 0);
        if (id > 0)
        {
            int __id = 0;
            for (auto it = begin(getInstance()->stringsDesc); it != end(getInstance()->stringsDesc); ++it)
            {
                auto _str = (*it.base());
                if (id == _str->getId())
                {
                    delete (_str);
                    auto str = new StringDescriptor(pointer, id);
                    getInstance()->stringsDesc.at(__id) = str;
                    return __id;
                }
                __id++;
            }
            return id;
        }
        else
        {
            uint8_t last = 0;
            for (auto str : getInstance()->stringsDesc)
            {
                auto _id = str->getId();
                last = _id > last ? _id : last;
            }
            last++;
            auto str = new StringDescriptor(pointer, last);
            getInstance()->stringsDesc.push_back(str);
            return last;
        }

        return -1;
    }

    USBInterface *USBdevice::addInterface()
    {
        assert(_interfaces.size() < 6);
        auto intf = new USBInterface();
        _interfaces.push_back(intf);

        return intf;
    }

    void USBdevice::_onMount()
    {
        printf("onMount\n");
    }

    void USBdevice::_onUnmount()
    {
        // printf("onUnmount\n");
    }

    void USBdevice::_onSuspend(bool remote_wakeup_en)
    {
        // printf("onSuspend\n");
    }

    void USBdevice::_onResume()
    {
        printf("onResume\n");
    }

}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
__attribute__((weak)) void tud_mount_cb(void)
{
    auto device = esptinyusb::USBdevice::getInstance();
    device->_onMount();
}

// Invoked when device is unmounted
__attribute__((weak)) void tud_umount_cb(void)
{
    auto device = esptinyusb::USBdevice::getInstance();
    device->_onUnmount();
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
__attribute__((weak)) void tud_suspend_cb(bool remote_wakeup_en)
{
    auto device = esptinyusb::USBdevice::getInstance();
    device->_onSuspend(remote_wakeup_en);
}

// Invoked when usb bus is resumed
__attribute__((weak)) void tud_resume_cb(void)
{
    auto device = esptinyusb::USBdevice::getInstance();
    device->_onResume();
}

// ------------- descriptors ------------------//
__attribute__((weak)) uint8_t const *tud_descriptor_device_cb(void)
{
    auto descriptor = esptinyusb::USBdevice::getInstance()->getDeviceDescriptor();
    printf_buffer((uint8_t *)descriptor, 18);

    return descriptor;
}

/**
 * @brief Invoked when received GET CONFIGURATION DESCRIPTOR.
 * Descriptor contents must exist long enough for transfer to complete
 *
 * @param index
 * @return uint8_t const* Application return pointer to descriptor
 */
__attribute__((weak)) uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index; // for multiple configurations
    auto descriptor = esptinyusb::USBdevice::getInstance()->getConfigDescriptor();

    return descriptor;
}

/**
 * @brief Invoked when received GET STRING DESCRIPTOR request.
 * Application returns pointer to descriptor, whose contents must exist long
 * enough for transfer to complete
 *
 * @param index
 * @return uint16_t const*
 */
__attribute__((weak)) uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    auto descriptors = esptinyusb::USBdevice::getInstance();
    return descriptors->getStringDescriptor(index, langid);
}

// #endif // CONFIG_TINYUSB
