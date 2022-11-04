# Description

This is very early version and still WiP, so API may and probably will change. All implemented devices are provided with simple examples for arduino.
Library is compatible with arduino, platformio, esp-idf and there will be also repository with examples for esp-idf.

I want to replace old callbacks build on `class callback` to use `std::function`, which also let us to use lambdas. In the future i maybe add C style callbacks too.
I wanted to make this library as much flexible as possible, so all tinyusb callbacks are weak and its possible to override any in user code (advanced users only).


Library allows to build USB class devices and to make it usable with minimal effort:
- [x] CDC, communication device class,
- [x] MSC, mass storage class,
- [x] HID, human interface device class: keyboard, mouse, generic IN/OUT, option to combine few reports
- [ ] MIDI, musical instrument digital interface class,
- [ ] DFU, device firmware update class,- its still not ready
- [x] WebUSB, its using vendor class to show webusb usage.

# How to
Library allows to set all values in standard USB device like:
- manufacturer
- product name
- serial number
- revision
- VID and PID

```
ANYusb device;  // any USB class like HID, MSC, CDC
device.mfgStringId(addString(""));
device.productStringId(addString(""));
device.serialStringId(addString(""));
device.setVidPid(vid, pid);
device.setVersion(revisionBCD, usbBCD);
```

# Todo
There is still a lot to do, so any suggestions are welcome.

# Documentation
I would like to finish this project with fancy documentation, but this is time consuming and i have full time job and few more projects which i would like to continue, so any help is appreciated

https://tinyusb.esp32.eu.org/

# Contributions
Issues and PRs are welcome.

# Hardware
To use native USB we need to connect pins 19 and 20 to usb cable or with similar connectors:
![](https://ae01.alicdn.com/kf/HTB1MFvqNgHqK1RjSZJnq6zNLpXaR/10-sztuk-Mini-Micro-USB-do-DIP-2-54mm-Adapter-z-cze-modu-u-Panel-kobiet.jpg)
![](https://ae01.alicdn.com/kf/HTB1cfmCgcnI8KJjSspeq6AwIpXa6/AMS1117-3-3V-AMS1117-3-3V-Mini-USB-5V-3-3V-DC-Perfect-Power-Supply-Module.jpg)
