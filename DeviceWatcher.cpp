#include "DeviceWatcher.h"
#include <string.h>

namespace MMNL {
	void DeviceWatcher::init(uint16_t local_ard) {
		max_nmb = min_nmb = local_ard;
		dev_count = 1;
	}

	void DeviceWatcher::new_dev(uint16_t adr) {
		if (adr > max_nmb)
			max_nmb = adr;
		if (adr < min_nmb)
			min_nmb = adr;
		if (adr == midle_nmb)
			midle_nmb = 0;
		dev_count++;
		if (dev_count > max_nmb - min_nmb)
			dev_count = max_nmb - min_nmb + 1;
	}

	void DeviceWatcher::remove_dev(uint16_t adr)
	{
		if (adr == max_nmb)
			--max_nmb;
		else if (adr == min_nmb)
			min_nmb++;
		else if (adr > min_nmb && adr < max_nmb) 
			if (midle_nmb == 0 || adr < midle_nmb)
				midle_nmb = adr;
		dev_count--;
	}

	void DeviceWatcher::save_structure(uint8_t* buf){
		memcpy(buf + 0, &max_nmb, 2);
		memcpy(buf + 2, &min_nmb, 2);
		memcpy(buf + 4, &midle_nmb, 2);
		memcpy(buf + 6, &dev_count, 2);
	}

	void DeviceWatcher::load_structure(uint8_t* buf){
		memcpy(&max_nmb, buf + 0, 2);
		memcpy(&min_nmb, buf + 2, 2);
		memcpy(&midle_nmb, buf + 4, 2);
		memcpy(&dev_count, buf + 6, 2);
	}

	uint16_t DeviceWatcher::get_next_adr() {
		if (midle_nmb != 0)
			return midle_nmb;
		else if (max_nmb < 2046)
			return max_nmb + 1;
		else if (min_nmb > 1)
			return min_nmb - 1;
		else
			return 0;
	}

	uint16_t DeviceWatcher::get_count()
	{
		return dev_count;
	}
};