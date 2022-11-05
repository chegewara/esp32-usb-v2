#pragma once

#if CONFIG_TINYUSB
#if CONFIG_TINYUSB_MSC_ENABLED

namespace esptinyusb
{

    class HIDreport
    {
    private:
    public:
        HIDreport();
        ~HIDreport();
    };

    HIDreport::HIDreport()
    {
    }

    HIDreport::~HIDreport()
    {
    }

} // namespace esptinyusb

#endif // CONFIG_TINYUSB_MSC_ENABLED
#endif // CONFIG_TINYUSB
