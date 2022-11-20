#include "../usb_hid.hpp"

static std::vector<HIDdevice*> _hid_devices;
namespace esptinyusb
{

    HIDdevice::~HIDdevice()
    {
    }

    bool HIDdevice::begin(uint8_t eps, uint8_t* desc, size_t len)
    {
        if(len < 0) return;
        auto intf = addInterface();
        intf->claimInterface();
        intf->addEndpoint(eps);

        stringIndex = addString("CONFIG_TINYUSB_DESC_HID_STRING", -1);

        uint8_t tmp[] = {TUD_HID_INOUT_DESCRIPTOR((uint8_t)intf->ifIdx, (uint8_t)stringIndex, HID_ITF_PROTOCOL_NONE, desc_hid_report.size(), (uint8_t)(intf->endpoints.at(0)->epId), (uint8_t)(0x80 | intf->endpoints.at(0)->epId), report_len, 10)};

        if(len > 0)
            intf->setDesc(desc, len));
        else
            intf->setDesc(tmp, sizeof(tmp));

        _hid_devices.push_back(this);
        return true;
    }

} // namespace esptinyusb



// Invoked when received GET HID REPORT DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
__attribute__ ((weak)) uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance)
{
    if(_hid_devices.at(instance))
        return _hid_devices.at(instance)->getHidReport();
    return NULL;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
__attribute__ ((weak)) uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) itf;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
__attribute__ ((weak)) void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    auto hid = _hid_devices.at(instance);
    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {
        hid->_onSetReport(report_id, buffer, bufsize);
    }
}

// Invoked when received SET_PROTOCOL request
// protocol is either HID_PROTOCOL_BOOT (0) or HID_PROTOCOL_REPORT (1)
// TU_ATTR_WEAK void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol);

// Invoked when received SET_IDLE request. return false will stall the request
// - Idle Rate = 0 : only send report if there is changes, i.e skip duplication
// - Idle Rate > 0 : skip duplication, but send at least 1 report every idle rate (in unit of 4 ms).
// TU_ATTR_WEAK bool tud_hid_set_idle_cb(uint8_t instance, uint8_t idle_rate);

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
TU_ATTR_WEAK void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len)
{
    auto hid = _hid_devices.at(instance);
    hid->_onSendComplete(report, len);
}

