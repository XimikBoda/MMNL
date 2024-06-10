#pragma once
#include "MMNL_Conf.h"

namespace MMNL {
	class PacketIndexWatcher {
#ifdef MMNL_PIW_FULL_LIST
		uint8_t full_ind[2048];
#endif
		uint16_t last_dev_adr[MMNL_PIW_LAST_LIST_SIZE];
		uint16_t last_dev_ind[MMNL_PIW_LAST_LIST_SIZE];
		uint16_t last_dev_size = 0;

		uint16_t find_pos(uint16_t adr);
		void move_down(uint16_t to);
		bool ind_is_good(uint16_t old, uint16_t new_ind);

	public:
		PacketIndexWatcher();

		bool is_good(uint16_t adr, uint8_t ind, bool overwrite = false);

		void reset();
	};
};