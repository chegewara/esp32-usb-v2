#pragma once
#include "esp_vfs_fat.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"

#include "usb_msc.hpp"

namespace esptinyusb
{
    class USBflash : public USBMSC
    {
    public:
        const char *base_path = "/fatfs";
        const char *partition_label = NULL;
        wl_handle_t wl_handle = -1;
        bool sdcardReady = false;

    public:
        USBflash();
        ~USBflash();
        virtual bool end();

        virtual void partition(const char *path, const char *label);
    };

} // namespace esptinyusb

// static uint8_t _buf[4 * 1024] = {};

// class FlashCallbacks : public MSCCallbacks {
//     FlashUSB* m_parent;
//     wl_handle_t wl_handle_thandle;
//     int s_disk_block_size = 0;
//     int s_pdrv = 0;
//     bool eject = true;

// public:
//     FlashCallbacks(FlashUSB* ram, wl_handle_t handle, int pdrv) { m_parent = ram; wl_handle_thandle = handle; s_pdrv = pdrv; }
//     ~FlashCallbacks() { }
//     void onInquiry(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
//     {
//         if (m_parent->m_private)
//         {
//             m_parent->m_private->onInquiry(lun, vendor_id, product_id, product_rev);
//         } else {
//             const char vid[] = "ESP32-S2";
//             const char pid[] = "FLASH";
//             const char rev[] = "1.0";

//             memcpy(vendor_id  , vid, strlen(vid));
//             memcpy(product_id , pid, strlen(pid));
//             memcpy(product_rev, rev, strlen(rev));
//             log_v("default onInquiry");
//         }
//     }



//     bool onStop(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
//     {
//         if (m_parent->m_private)
//         {
//             return m_parent->m_private->onStop(lun, power_condition, start, load_eject);
//         } else {
//             (void) lun;
//             (void) power_condition;

//             if ( load_eject )
//             {
//                 if (!start) {
//                     // Eject but first flush.
//                     if (disk_ioctl(s_pdrv, CTRL_SYNC, NULL) != RES_OK) {
//                         eject = false;
//                         return false;
//                     } else {
//                         eject = true;
//                     }
//                 } else {
//                     // We can only load if it hasn't been ejected.
//                     return !eject;
//                 }
//             } else {
//                 if (!start) {
//                     // Stop the unit but don't eject.
//                     if (disk_ioctl(s_pdrv, CTRL_SYNC, NULL) != RES_OK) {
//                         return false;
//                     }
//                 }
//             }

//             return true;
//         }
//     }
//     int32_t onRead(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
//     {
//         if (m_parent->m_private)
//         {
//             return m_parent->m_private->onRead(lun, lba, offset, buffer, bufsize);
//         } else {
//             log_v("default onread");
//             (void) lun;
//             if(CONFIG_WL_SECTOR_SIZE > CONFIG_TINYUSB_MSC_BUFSIZE){
//                 disk_read(s_pdrv, _buf, lba, 1);
//                 memcpy(buffer, &_buf[offset], bufsize);
//             } else {
//                 disk_read(s_pdrv, (BYTE*)buffer, lba, 1);
//             }

//             return bufsize;
//         }
//     }

//     int32_t onWrite(uint8_t lun, uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
//     {
//         if (m_parent->m_private)
//         {
//             return m_parent->m_private->onWrite(lun, lba, offset, buffer, bufsize);
//         } else {
//             log_v("default onwrite; lba: %d, off: %d, size: %d", lba, offset, bufsize);
//             (void) lun;
//             if(CONFIG_WL_SECTOR_SIZE > CONFIG_TINYUSB_MSC_BUFSIZE){
//                 disk_read(s_pdrv, _buf, lba, 1);
//                 memcpy(&_buf[offset], buffer, bufsize);
//                 disk_write(s_pdrv, _buf, lba, 1);
//             } else {
//                 disk_write(s_pdrv, (BYTE*)buffer, lba, 1);
//             }
//             log_v("lba: %d, offset: %d, buffsize: %d, s_disk: %d, count: %d ", lba, offset, bufsize, s_disk_block_size, 1);

//             return bufsize;
//         }
//     }
// };

// FlashUSB::FlashUSB(bool _aes)
// {
//     static int pdrv = 0;

//     MSCusb::setCallbacks(new FlashCallbacks(this, wl_handle, pdrv));
//     pdrv++;
//     encrypted = _aes;
// }


