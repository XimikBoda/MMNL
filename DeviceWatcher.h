#pragma once
#include "MMNL_Conf.h"

namespace MMNL {
	class DeviceWatcher {
		uint16_t max_nmb = 0;
		uint16_t min_nmb = 0;
		uint16_t midle_nmb = 0;
		uint16_t dev_count = 0;
	public:

		void init(uint16_t local_ard);

		void new_dev(uint16_t adr);

		void remove_dev(uint16_t adr);

		void save_structure(uint8_t* buf);

		void load_structure(uint8_t* buf);

		uint16_t get_next_adr();

		uint16_t get_count();
	};
};