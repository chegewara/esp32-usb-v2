#include "usb_webusb.hpp"

// #if CONFIG_TINYUSB
// #if CFG_TUD_VENDOR

static esptinyusb::USBWebSerial *_webserial = nullptr;

namespace esptinyusb
{
    bool USBWebSerial::begin(uint8_t eps)
    {
        _webserial = this;
        auto intf = addInterface();
        ifIdx = intf->claimInterface();

        intf->addEndpoint(eps);
        intf->addEndpoint(eps);

        stringIndex = addString(CONFIG_TINYUSB_DESC_VENDOR_STRING, -1);

        // Interface number, string index, attributes, detach timeout, transfer size
        uint8_t tmp[] = {TUD_VENDOR_DESCRIPTOR((uint8_t)intf->ifIdx, (uint8_t)stringIndex, intf->endpoints.at(0)->epId, (uint8_t)(0x80 | intf->endpoints.at(1)->epId), 64)};
        intf->setDesc(tmp, sizeof(tmp));

        // if ("url" != nullptr)
            landingPageURI("https://esp32.eu.org");

        return true;
    }

    int USBWebSerial::available(void)
    {
        return tud_vendor_n_available(ifIdx);
    }

    int USBWebSerial::peek(void)
    {
        int pos;
        uint8_t buffer[1];
        if (_is_connected)
        {
            tud_vendor_n_peek(ifIdx, buffer);
            return buffer[0];
        }

        return -1;
    }

    int USBWebSerial::read(void)
    {
        if (_is_connected)
        {
            char c;
            if (tud_vendor_n_available(ifIdx))
            {
                uint32_t count = tud_vendor_n_read(ifIdx, &c, 1);
                return c;
            }
        }

        return -1;
    }

    size_t USBWebSerial::read(uint8_t *buffer, size_t size)
    {
        if (_is_connected)
        {
            if (tud_vendor_n_available(ifIdx))
            {
                uint32_t count = tud_vendor_n_read(ifIdx, buffer, size);
                return count;
            }
        }

        return -1;
    }

    void USBWebSerial::flush(void)
    {
        tud_vendor_n_read_flush(ifIdx);
    }

    size_t USBWebSerial::write(uint8_t c)
    {
        return write(&c, 1);
    }

    size_t USBWebSerial::write(const uint8_t *buffer, size_t size)
    {
        uint32_t sent = tud_vendor_n_write(ifIdx, buffer, size);
        return sent;
    }

    USBWebSerial::operator bool() const
    {
        return _is_connected;
    }

    void USBWebSerial::_onData()
    {
        if (onReadCb)
            onReadCb();
    }

    void USBWebSerial::_onConnect(bool connected)
    {
        _is_connected = connected;
        if (onConnCb)
            onConnCb(connected);
    }

    uint8_t *USBWebSerial::getUrl()
    {
        return _url;
    }

    size_t USBWebSerial::getLength()
    {
        if (_url == nullptr)
            return 0;

        return _url[0];
    }

    void USBWebSerial::landingPageURI(const char *url, bool ssl)
    {
        if (_url != NULL)
        {
            free(_url);
            _url = NULL;
        }
        size_t size = strlen(url);
        _url = (uint8_t *)calloc(1, size + 3);
        if (_url)
        {
            memcpy(&_url[3], url, size);
            _url[0] = size + 3;
            _url[1] = 3;
            _url[2] = ssl;
        }
    }

} // namespace esptinyusb

void tud_vendor_rx_cb(uint8_t itf)
{
    _webserial->_onData();
}

//--------------------------------------------------------------------+
// WebUSB use vendor class
//--------------------------------------------------------------------+
extern "C"
{
#define BOS_TOTAL_LEN \
    (TUD_BOS_DESC_LEN + TUD_BOS_WEBUSB_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

    uint8_t const desc_bos[] = {
        // total length, number of device caps
        TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 2),

        // Vendor Code, iLandingPage
        TUD_BOS_WEBUSB_DESCRIPTOR(VENDOR_REQUEST_WEBUSB, 1),

        // Microsoft OS 2.0 descriptor
        TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, VENDOR_REQUEST_MICROSOFT)};

    uint8_t const *tud_descriptor_bos_cb(void)
    {
        if (_webserial == NULL)
            return nullptr;

        return desc_bos;
    }

    // Invoked when received VENDOR control request
    bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
    {
        if (_webserial == NULL)
            return false;
        // nothing to with DATA & ACK stage
        if (stage != CONTROL_STAGE_SETUP)
            return true;

        switch (request->bRequest)
        {
        case VENDOR_REQUEST_WEBUSB:
            // match vendor request in BOS descriptor
            // Get landing page url
            if (!_webserial->getUrl())
                return false;
            return tud_control_xfer(rhport, request, (void *)_webserial->getUrl(), _webserial->getLength());

        case VENDOR_REQUEST_MICROSOFT:
            if (request->wIndex == 7)
            {
                // Get Microsoft OS 2.0 compatible descriptor
                uint16_t total_len;
                memcpy(&total_len, desc_ms_os_20 + 8, 2);

                return tud_control_xfer(rhport, request, (void *)desc_ms_os_20, total_len);
            }
            else
            {
                return false;
            }
            break;
        case 0x22:
            // Webserial simulate the CDC_REQUEST_SET_CONTROL_LINE_STATE (0x22) to
            // connect and disconnect.
            _webserial->_onConnect(request->wValue != 0);

            // response with status OK
            return tud_control_status(rhport, request);

        default:
            // stall unknown request
            return false;
        }

        return true;
    }
}

// #endif // CFG_TUD_VENDOR
// #endif // CONFIG_TINYUSB
