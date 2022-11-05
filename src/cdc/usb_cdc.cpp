
// #include "usb_descriptors.hpp"
#include "usb_device.hpp"
#include "descriptor.hpp"

#include "usb_cdc.hpp"

#if CONFIG_TINYUSB
#if CFG_TUD_CDC

namespace esptinyusb
{

    static std::vector<USBCDC *> _intfs;
    uint8_t USBCDC::_port = 0;

    USBCDC::USBCDC()
    {
    }

    USBCDC::~USBCDC()
    {
        printf("destructor\n\n");
    }

    bool USBCDC::begin(uint8_t eps)
    {
        assert(_port < 2);
        port = _port++; // to use 2 CDC-ACM ports
        auto intf = addInterface();    // we need to create 2 interfaces, even if later descriptor is built all in one, and the one is just a dummy interface
        auto intf1 = addInterface();
        intf->claimInterface();
        intf1->claimInterface();

        intf->addEndpoint(eps);
        intf1->addEndpoint(eps + 1);
        intf1->addEndpoint(eps + 1);

        stringIndex = addString(CONFIG_TINYUSB_DESC_CDC_STRING);
 
        uint8_t tmp[] = {TUD_CDC_DESCRIPTOR((uint8_t)intf->ifIdx, (uint8_t)stringIndex, (uint8_t)(0x80 | (intf->endpoints.at(0)->epId)), 8, intf1->endpoints.at(0)->epId, (uint8_t)(0x80 | intf1->endpoints.at(1)->epId), 64)};
        intf->setDesc(tmp, sizeof(tmp));

        _intfs.push_back(this); // push to vector for multi CDC ports (WiP)
        return true;
    }

    bool USBCDC::end()
    {
        // TODO
        return true;
    }

}

// Invoked when received new data
__attribute__((weak)) void tud_cdc_rx_cb(uint8_t itf)
{
    auto cdc = esptinyusb::_intfs.at(itf);
    if (cdc)
        cdc->onData();
}

// Invoked when received `wanted_char`
__attribute__((weak)) void tud_cdc_rx_wanted_cb(uint8_t itf, char wanted_char)
{
    // auto cdc = esptinyusb::USBCDC::getInstance(itf);
    // cdc->onData();
}

// Invoked when space becomes available in TX buffer
__attribute__((weak)) void tud_cdc_tx_complete_cb(uint8_t itf)
{
    // auto cdc = esptinyusb::USBCDC::getInstance(itf);
    // cdc->onComplete();
}

// Invoked when line state DTR & RTS are changed via SET_CONTROL_LINE_STATE
__attribute__((weak)) void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    // auto cdc = esptinyusb::USBCDC::getInstance(itf);
    // cdc->onLineState(dtr, rts);
}

// Invoked when line coding is change via SET_LINE_CODING
__attribute__((weak)) void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const *p_line_coding)
{
    // auto cdc = esptinyusb::USBCDC::getInstance(itf);
    // cdc->onLineCoding(p_line_coding);
}

// Invoked when received send break
__attribute__((weak)) void tud_cdc_send_break_cb(uint8_t itf, uint16_t duration_ms)
{
    // auto cdc = esptinyusb::USBCDC::getInstance(itf);
    // cdc->onData();
}

#endif // CFG_TUD_CDC
#endif // CONFIG_TINYUSB
