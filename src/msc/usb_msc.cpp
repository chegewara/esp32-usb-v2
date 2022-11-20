#include "private/descriptor.hpp"

#include "usb_msc.hpp"

// #if CONFIG_TINYUSB
// #if CONFIG_TINYUSB_MSC_ENABLED

#include "esp_vfs_fat.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"
static std::vector<esptinyusb::USBmsc *> _device;

namespace esptinyusb
{
	uint8_t USBmsc::_luns = 0;
	uint8_t USBmsc::_pdrvs = 0;

	USBmsc::USBmsc()
	{
		_device.push_back(this);
		_callbacks = new USBMSCcallbacks();
		_lun = _luns++;
		_pdrv = _pdrvs;
	}

	USBmsc::~USBmsc()
	{
		printf("destructor\n\n");

		// if (_callbacks)
		// 	delete (_callbacks);
	}

	bool USBmsc::begin(uint8_t eps)
	{
		auto intf = addInterface();
		auto ifIdx = intf->claimInterface();

		intf->addEndpoint(eps);

		stringIndex = addString(CONFIG_TINYUSB_DESC_MSC_STRING, -1);

		uint8_t tmp[] = {TUD_MSC_DESCRIPTOR((uint8_t)intf->ifIdx, (uint8_t)stringIndex, intf->endpoints.at(0)->epId, (uint8_t)(0x80 | intf->endpoints.at(0)->epId), 64)}; // highspeed 512
		intf->setDesc(tmp, sizeof(tmp));

		return true;
	}

	bool USBmsc::end()
	{
		return true;
	}

	void USBmsc::_onInquiry(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
	{
		_callbacks->onInquiry(lun, vendor_id, product_id, product_rev);
	}

	bool USBmsc::_onReady(uint8_t lun)
	{
		return _callbacks->onReady(lun);
	}

	void USBmsc::_onCapacity(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
	{
		_callbacks->onCapacity(lun, block_count, block_size);
	}

	bool USBmsc::_onStop(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
	{
		return _callbacks->onStop(lun, power_condition, start, load_eject);
	}

	int32_t USBmsc::_onRead(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
	{
		return _callbacks->onRead(lun, lba, offset, buffer, bufsize);
	}

	int32_t USBmsc::_onWrite(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
	{
		return _callbacks->onWrite(lun, lba, offset, buffer, bufsize);
	}

}

TU_ATTR_WEAK void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
	for (auto d : _device)
	{
		if (d->luns() == lun)
		{
			d->_onInquiry(lun, vendor_id, product_id, product_rev);
		}
	}
}

TU_ATTR_WEAK bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
	for (auto d : _device)
	{
		if (d->luns() == lun)
		{
			return d->_onReady(lun);
		}
	}
	return false;
}

TU_ATTR_WEAK void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
	for (auto d : _device)
	{
		if (d->luns() == lun)
		{
			d->_onCapacity(lun, block_count, block_size);
		}
	}
}

TU_ATTR_WEAK bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
	for (auto d : _device)
	{
		if (d->luns() == lun)
		{
			return d->_onStop(lun, power_condition, start, load_eject);
		}
	}
	return false;
}

TU_ATTR_WEAK int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
	for (auto d : _device)
	{
		if (d->luns() == lun)
		{
			return d->_onRead(lun, lba, offset, buffer, bufsize);
		}
	}
	return -1;
}

TU_ATTR_WEAK int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
	for (auto d : _device)
	{
		if (d->luns() == lun)
		{
			return d->_onWrite(lun, lba, offset, buffer, bufsize);
		}
	}
	return -1;
}

TU_ATTR_WEAK int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{

	void const *response = NULL;
	int16_t resplen = 0;

	// most scsi handled is input
	bool in_xfer = true;

	switch (scsi_cmd[0])
	{
	case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
		// Host is about to read/write etc ... better not to disconnect disk
		resplen = 0;
		break;
	/// @bug this is causing ramdisk crash on disk removal 
	case 0x35:
	{
			auto pdrv = _device.at(lun)->pdrv();
			if (disk_ioctl(pdrv, CTRL_SYNC, NULL) != RES_OK)
			{
				printf("failed to sync\n");
			}
		break;
	}
	default:
		// Set Sense = Invalid Command Operation
		tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

		// negative means error -> tinyusb could stall and/or response with failed status
		resplen = -1;
		break;
	}

	// return resplen must not larger than bufsize
	if (resplen > bufsize)
		resplen = bufsize;

	if (response && (resplen > 0))
	{
		if (in_xfer)
		{
			memcpy(buffer, response, resplen);
		}
		else
		{
			// SCSI output
		}
	}

	return resplen;
}

// Support multi LUNs
TU_ATTR_WEAK uint8_t tud_msc_get_maxlun_cb(void)
{
	return esptinyusb::USBmsc::_luns;
}

// Invoked when Read10 command is complete
TU_ATTR_WEAK void tud_msc_read10_complete_cb(uint8_t lun)
{
}
// Invoke when Write10 command is complete, can be used to flush flash caching
TU_ATTR_WEAK void tud_msc_write10_complete_cb(uint8_t lun)
{
	for (auto d : _device)
	{
		if (d->luns() == lun)
		{
			if (disk_ioctl(d->pdrv(), CTRL_SYNC, NULL) != RES_OK)
			{
				printf("failed to sync\n");
			}
		}
	}
}

// #endif // CONFIG_TINYUSB_MSC_ENABLED
// #endif // CONFIG_TINYUSB
