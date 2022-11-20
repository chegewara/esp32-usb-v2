#pragma once
#include <memory>
#include <functional>
#include "private/usb_device.hpp"

// #if CONFIG_TINYUSB
// #if CFG_TUD_VENDOR

namespace esptinyusb
{
    /**
     * @brief WebUSB class
     * 
     */
    class USBWebSerial : public BaseDevice
    {
        typedef void (*read_cb_t)();
        typedef void (*connect_cb_t)(bool);

    protected:
        uint8_t port = 0;
        static uint8_t _port;
        uint8_t *_url = nullptr;
        bool _is_connected = false;

    public:
        using BaseDevice::BaseDevice;
        ~USBWebSerial() {}

        virtual bool begin(uint8_t _eps = 1);
        virtual bool end() { return true; }

        /**
         * @brief Checks if any bytes are available to read from fifo
         * 
         * @return int number of bytes in fifo
         */
        virtual int available(void);
        /**
         * @brief Read one byte from fifo without removing it
         * 
         * @return int return read byte or -1 if failed
         */
        virtual int peek(void);
        /**
         * @brief Read 1 byte from fifo
         * 
         * @return int 
         */
        virtual int read(void);
        /**
         * @brief Read number of bytes from fifo
         * 
         * @param[in] buffer buffer to read bytes into
         * @param[in] size size of buffer
         * @return size_t number of read bytes 
         */
        virtual size_t read(uint8_t *buffer, size_t size);
        /**
         * @brief Flush all remaining bytes from read fifo
         * 
         */
        virtual void flush(void);
        /**
         * @brief Write one byte to fifo
         * 
         * @return size_t 
         */
        virtual size_t write(uint8_t);
        /**
         * @brief Write number of bytes to fifo
         * 
         * @param buffer 
         * @param size 
         * @return size_t 
         */
        virtual size_t write(const uint8_t *buffer, size_t size);
        operator bool() const;
        /**
         * @brief Add callback which will be called when data is received from host
         * 
         * @param cb 
         */
        virtual void onData(read_cb_t cb) { onReadCb = cb; }
        virtual void _onData();
        /**
         * @brief Adds callback which will be called when connection or disconnection with host is detected
         * 
         * @param cb 
         */
        virtual void onConnect(connect_cb_t cb)  { onConnCb = cb; }
        virtual void _onConnect(bool);

        /**
         * @brief Set landing page for webUSB protocol
         * 
         * @param url landing page url
         * @param ssl use landing page with http or https protocol
         */
        virtual void landingPageURI(const char *url, bool ssl = true);
        virtual uint8_t* getUrl();
        virtual size_t getLength();

    protected:
        read_cb_t onReadCb = nullptr;
        connect_cb_t onConnCb = nullptr;
    };

}

#define MS_OS_20_DESC_LEN 0xB2
#define EPNUM_VENDOR 0x04
#define _vendor  "Vendor class (webUSB)"

// https://developers.google.com/web/fundamentals/native-hardware/build-for-webusb#microsoft_os_compatibility_descriptors
const uint8_t desc_ms_os_20[] = {
    // Set header: length, type, windows version, total length
    U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR),
    U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(MS_OS_20_DESC_LEN),

    // Configuration subset header: length, type, configuration index, reserved,
    // configuration total length
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION),
    0, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A),

    // Function Subset header: length, type, first interface, reserved, subset
    // length
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION),
    0 /*itf num*/, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08),

    // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub
    // compatible ID
    U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W',
    'I', 'N', 'U', 'S', 'B', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, // sub-compatible

    // MS OS 2.0 Registry property descriptor: length, type
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 0x0A - 0x08 - 0x08 - 0x14),
    U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY), U16_TO_U8S_LE(0x0007),
    U16_TO_U8S_LE(0x002A), // wPropertyDataType, wPropertyNameLength and
                           // PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00,
    'n', 0x00, 't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00,
    'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00,
    0x00,
    U16_TO_U8S_LE(0x0050), // wPropertyDataLength
    // bPropertyData: “{975F44D9-0D08-43FD-8B3E-127CA8AFFF9D}”.
    '{', 0x00, '9', 0x00, '7', 0x00, '5', 0x00, 'F', 0x00, '4', 0x00, '4', 0x00,
    'D', 0x00, '9', 0x00, '-', 0x00, '0', 0x00, 'D', 0x00, '0', 0x00, '8', 0x00,
    '-', 0x00, '4', 0x00, '3', 0x00, 'F', 0x00, 'D', 0x00, '-', 0x00, '8', 0x00,
    'B', 0x00, '3', 0x00, 'E', 0x00, '-', 0x00, '1', 0x00, '2', 0x00, '7', 0x00,
    'C', 0x00, 'A', 0x00, '8', 0x00, 'A', 0x00, 'F', 0x00, 'F', 0x00, 'F', 0x00,
    '9', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00};

enum
{
  VENDOR_REQUEST_WEBUSB = 1,
  VENDOR_REQUEST_MICROSOFT = 2
};


// #endif // CFG_TUD_VENDOR
// #endif // CONFIG_TINYUSB
