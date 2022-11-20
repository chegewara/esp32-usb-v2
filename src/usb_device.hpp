#pragma once
#include <stdio.h>
#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "common/tusb_common.h"
#include "tusb.h"
#include "descriptor.hpp"
#include "interface.hpp"

// #if CONFIG_TINYUSB

#ifndef CONFIG_TINYUSB_DESC_MANUFACTURER_STRING
#define CONFIG_TINYUSB_DESC_MANUFACTURER_STRING "manufacturer"
#endif
#ifndef CONFIG_TINYUSB_DESC_PRODUCT_STRING
#define CONFIG_TINYUSB_DESC_PRODUCT_STRING "product"
#endif
#ifndef CONFIG_TINYUSB_DESC_SERIAL_STRING
#define CONFIG_TINYUSB_DESC_SERIAL_STRING "serial"
#endif

/*
 * USB Persistence API
 * */
typedef enum
{
    RESTART_NO_PERSIST,
    RESTART_PERSIST,
    RESTART_BOOTLOADER,
    RESTART_BOOTLOADER_DFU,
    RESTART_TYPE_MAX
} restart_type_t;

namespace esptinyusb
{
    void persistentReset(restart_type_t _usb_persist_mode);

    class USBdevice : virtual DeviceDescriptor
    {
    private:
        static std::shared_ptr<USBdevice> _instance;
        bool isEnabled = false;
        TaskHandle_t usbTaskHandle = nullptr;
        std::vector<USBInterface *> _interfaces;
        std::vector<StringDescriptor *> stringsDesc;
        uint8_t *buffer = nullptr;
        tusb_desc_device_t _desc = DEVICE_DESCRIPTOR;

        void _init_tusb();
        void _init_hardware();

        void setDefaultDescriptorStrings();

    protected:
        /**
         * @brief Initialize USB hardware and create tud_task
         * 
         * @return true 
         * @return false 
         */
        virtual bool init() final;
        friend class BaseDevice;

    public:
        USBdevice() { assert(_instance == nullptr); }
        USBdevice(USBdevice &) = delete;
        ~USBdevice();
        /**
         * @brief Get the Instance 
         *      we want to make USBdevice class a singleton, so all USB class are inheriting from it 
         * 
         * @return std::shared_ptr<USBdevice> 
         */
        static std::shared_ptr<USBdevice> getInstance();
        /**
         * @brief Get the Config Descriptor
         * 
         * @return uint8_t* 
         */
        virtual uint8_t *getConfigDescriptor() final;
        /**
         * @brief Get the Device Descriptor 
         * 
         * @return uint8_t* 
         */
        virtual uint8_t *getDeviceDescriptor() final;
        /**
         * @brief Get the String Descriptor 
         * 
         * @param index 
         * @param langid 
         * @return const uint16_t* 
         */
        const uint16_t *getStringDescriptor(uint8_t index, uint16_t langid);
        /**
         * @brief Add the string for device descriptor and device class
         * 
         * @param pointer 
         * @param id 
         * @return uint8_t 
         */
        virtual uint8_t addString(const char *pointer, int8_t id = -1);

        virtual USBInterface *addInterface();
        /**
         * @brief Set the device descriptor 
         * 
         * @param src 
         */
        virtual void setDescriptor(tusb_desc_device_t &&src)
        {
            memcpy(&_desc, &src, sizeof(tusb_desc_device_t));
        }
        /**
         * @brief Set the USB device descriptor class, subclass and protocol
         * 
         * @param _class 
         * @param subclass 
         * @param protocol 
         */
        virtual void setClass(uint16_t _class, uint16_t subclass, uint16_t protocol) final
        {
            getInstance()->_desc.bDeviceClass = _class;
            getInstance()->_desc.bDeviceSubClass = subclass;
            getInstance()->_desc.bDeviceProtocol = protocol;
        }
        /**
         * @brief Set the vid and pid
         * 
         * @param vid 
         * @param pid 
         */
        virtual void setVidPid(uint16_t vid, uint16_t pid) final
        {
            getInstance()->_desc.idVendor = vid;
            getInstance()->_desc.idProduct = pid;
        }
        /**
         * @brief Set the device and usb version in BCD 
         * 
         * @param device 
         * @param usb 
         */
        virtual void setVersion(uint16_t device = 0x100, uint16_t usb = 0x200) final
        {
            getInstance()->_desc.bcdDevice = device;
            getInstance()->_desc.bcdUSB = usb;
        }
        virtual void cdcInUse() final
        {
            // Use Interface Association Descriptor (IAD) for CDC
            // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
            getInstance()->setClass(TUSB_CLASS_MISC, MISC_SUBCLASS_COMMON, MISC_PROTOCOL_IAD);
        }
        /**
         * @brief Set the manufacturer string id
         * 
         * @param id 
         */
        virtual void mfgStringId(uint8_t id) final
        {
            getInstance()->_desc.iManufacturer = id;
        }
        /**
         * @brief Set the product string id
         * 
         * @param id 
         */
        virtual void productStringId(uint8_t id) final
        {
            getInstance()->_desc.iProduct = id;
        }
        /**
         * @brief Set the serial number string id
         * 
         * @param id 
         */
        virtual void serialStringId(uint8_t id) final
        {
            getInstance()->_desc.iSerialNumber = id;
        }

    private:
        /**
         * @brief @todo add callback
         *
         */
        virtual void _onMount();
        /**
         * @brief @todo add callback
         * 
         */
        virtual void _onUnmount();
        /**
         * @brief @todo add callback
         * 
         * @param remote_wakeup_en 
         */
        virtual void _onSuspend(bool remote_wakeup_en);
        /**
         * @brief @todo add callback
         * 
         */
        virtual void _onResume();

        friend void ::tud_mount_cb(void);
        friend void ::tud_umount_cb(void);
        friend void ::tud_suspend_cb(bool remote_wakeup_en);
        friend void ::tud_resume_cb(void);

        friend uint8_t const *::tud_descriptor_device_cb(void);
        friend uint8_t const *::tud_descriptor_configuration_cb(uint8_t index);
        friend uint16_t const *::tud_descriptor_string_cb(uint8_t index, uint16_t langid);

        // friend tusb_desc_device_t *tusb_get_active_desc(void);
        // friend char **tusb_get_active_str_desc(void);
        // friend void tusb_clear_descriptor(void);
    };

    /**
     * @brief Base class for all USB class devices, which will inherit from it
     * 
     */
    class BaseDevice : public USBdevice
    {
    protected:
        int8_t stringIndex = 0;
        std::shared_ptr<USBdevice> device = nullptr;

    public:
        int8_t ifIdx = -1;
        BaseDevice()
        {
            device = USBdevice::getInstance();
            device->init();
        }
        ~BaseDevice() = default;

        /**
         * @brief begin should add device descriptor and initialize interfaces
         *
         * @param _eps
         * @return true
         * @return false
         */
        virtual bool begin(uint8_t _eps = 1) = 0;
        virtual bool end() = 0;

        virtual USBInterface *addInterface() final
        {
            return device->addInterface();
        };
    };
}

// #endif // CONFIG_TINYUSB
