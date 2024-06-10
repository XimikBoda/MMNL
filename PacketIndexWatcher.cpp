#include "PacketIndexWatcher.h"

namespace MMNL {
	uint16_t PacketIndexWatcher::find_pos(uint16_t adr) {
		for (uint16_t i = 0; i < last_dev_size; ++i)
			if (last_dev_adr[i] == adr)
				return i;
		return 0xFFFF;
	}

	void PacketIndexWatcher::move_down(uint16_t to) {
		if (to >= MMNL_PIW_LAST_LIST_SIZE)
			return;

		for (uint16_t i = to; i > 0; --i)
			last_dev_adr[i] = last_dev_adr[i - 1],
			last_dev_ind[i] = last_dev_ind[i - 1];
	}

	PacketIndexWatcher::PacketIndexWatcher() {
		reset();
	}

	bool PacketIndexWatcher::ind_is_good(uint16_t old, uint16_t new_ind) {
		uint16_t w = old + 256 - 20;
		if (new_ind > old && new_ind < w || w >= 256 && new_ind < old - 20)
			return true;
		return false;
	}

	bool PacketIndexWatcher::is_good(uint16_t adr, uint8_t ind, bool overwrite) {
#ifdef MMNL_PIW_FULL_LIST
		if (MMNL_GET_NETWORK(adr) == MMNL_GET_NETWORK(local_ard))
			if (ind_is_good(full_ind[MMNL_GET_ADR(adr)], ind) || overwrite) {
				full_ind[MMNL_GET_ADR(adr)] = ind;
				return true;
			}
			else
				return false;
#endif
		if (MMNL_PIW_LAST_LIST_SIZE == 0)
			return true;

		uint16_t pos = find_pos(adr);

		if (pos == 0xFFFF) {
			if (last_dev_size < MMNL_PIW_LAST_LIST_SIZE)
				last_dev_size++;
			move_down(last_dev_size);
			last_dev_adr[0] = adr;
			last_dev_ind[0] = ind;
			return true;
		}

		if (ind_is_good(last_dev_ind[pos], ind) || overwrite) {
			if (pos != 0)
				move_down(pos);
			last_dev_adr[0] = adr;
			last_dev_ind[0] = ind;
			return true;
		}
		return false;
	}

	void PacketIndexWatcher::reset() {
#ifdef MMNL_PIW_FULL_LIST
		for (uint16_t i = 0; i < 2048; ++i)
			full_ind[i] = 255;
#endif
		last_dev_size = 0;
	}
};