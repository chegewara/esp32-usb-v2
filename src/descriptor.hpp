#pragma once
#include <stdio.h>
#include <string.h>
// #include "usb/usb_types_stack.h"
// #include "usb/usb_types_ch9.h"
#include "common/tusb_common.h"

#include <algorithm>
#include <iterator>

#include "interface.hpp"

// #if CONFIG_TINYUSB

namespace esptinyusb
{
#define DEVICE_DESCRIPTOR  {        \
        .bLength = sizeof(tusb_desc_device_t),      \
        .bDescriptorType = TUSB_DESC_DEVICE,        \
        .bcdUSB = 0x200,        \
        .bDeviceClass = 0x00,       \
        .bDeviceSubClass = 0x00,        \
        .bDeviceProtocol = 0x00,        \
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,      \
        .idVendor = 0x303a,     \
        .idProduct = 0xf001,        \
        .bcdDevice = 0x100,    \
        .iManufacturer = 0x01,      \
        .iProduct = 0x02,       \
        .iSerialNumber = 0x03,      \
        .bNumConfigurations = 0x01,     \
    }
    /**
     * @brief Interface to handle device descriptor
     * 
     */
    class DeviceDescriptor
    {
    protected:
        tusb_desc_device_t _desc = DEVICE_DESCRIPTOR;

    public:
        DeviceDescriptor() {}
        ~DeviceDescriptor() {}

        virtual void setDescriptor(tusb_desc_device_t &&src) = 0;
        // virtual uint8_t *getDescriptor() = 0;
        virtual void setClass(uint16_t _class, uint16_t subclass, uint16_t protocol) = 0;
        virtual void setVidPid(uint16_t vid, uint16_t pid) = 0;
        virtual void setVersion(uint16_t device = 0x100, uint16_t usb = 0x200) = 0;
        virtual void cdcInUse() = 0;
        virtual void mfgStringId(uint8_t id) = 0;
        virtual void productStringId(uint8_t id) = 0;
        virtual void serialStringId(uint8_t id) = 0;
    };

    /**
     * @brief USB device strings descriptor
     * 
     */
    class StringDescriptor
    {
    private:
        uint8_t _id;
        char _string[32] = {};

    public:
        StringDescriptor() = delete;
        virtual ~StringDescriptor(){}
        StringDescriptor(const char* str, uint8_t id) {
            strncpy(_string, str, 31);
            _id = id;
        }

        /**
         * @brief Get the Id objectGet string descriptor id
         * 
         * @return uint8_t id
         */
        virtual uint8_t getId() final { return _id; }
        /**
         * @brief Get the Descriptor 
         * 
         * @return const char* string descriptor value
         */
        virtual const char* getDescriptor() final { return _string; }
    };

}

// #endif // CONFIG_TINYUSB
