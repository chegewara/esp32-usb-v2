#pragma once
#include <stdio.h>
#include <string.h>
#include <vector>
#include <memory>

// #include "usb/usb_types_stack.h"
// #include "usb/usb_types_ch9.h"

#include "endpoint.hpp"

#if CONFIG_TINYUSB_ENABLED

namespace esptinyusb
{
    using namespace std;

    class USBInterface
    {
    public:
        int8_t ifIdx = -1;
        uint8_t *_desc = nullptr;
        size_t _desc_len = 0;
        vector<USBEndpoint *> endpoints;

    protected:
        static bool interfaces[6];

    public:
        USBInterface() {}
        ~USBInterface() {
            interfaces[ifIdx] = false;

            free(_desc);
            
            // for(auto ep : endpoints)
            // {
            //     // delete(ep);
            // }
        }

    public:
        virtual size_t getLength() {
            return _desc_len;
        }
        virtual const uint8_t* getDesc() {
            return _desc;
        }
        virtual void setDesc(uint8_t* desc, size_t len) {
            _desc_len = len;
            _desc = (uint8_t*)calloc(1, len);
            memcpy(_desc, desc, len);
        }

        virtual void addEndpoint(uint8_t ep)
        {
            auto _ep = new USBEndpoint(ep);
            endpoints.push_back(_ep);
        }
        virtual int8_t claimInterface(int8_t idx = -1)
        {
            ifIdx = idx;
            for (size_t i = 0; i < 6; i++)
            {
                if (interfaces[i] == false)
                {
                    interfaces[i] = true;
                    ifIdx = i;
                    return i;
                }
            }
            return ifIdx;
        }
    };

}

#endif // CONFIG_TINYUSB_ENABLED
