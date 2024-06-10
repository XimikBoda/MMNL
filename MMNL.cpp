#include "MMNL.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

namespace MMNL {
	bool MMNL::init(Module::Base** mlist_, uint8_t mlist_size_, const char* device_name, packet_callback pc) {
		mlist = mlist_;
		mlist_size = mlist_size_;
		dev_name = device_name;
		user_packet_callback = pc;
		rnd_v = (rand() & 0xFFFF) | ((rand() & 0xFFFF) << 16);
		for (int i = 0; i < mlist_size; ++i)
			if (mlist[i]->init())
				return true;
		return false;
	}

	void MMNL::send_buf() {
		for (int i = 0; i < mlist_size; ++i)
			mlist[i]->send(MMNL_buf, MMNL_buf_size);
	}

	void MMNL::pcocess_packet() {
		uint16_t d_adr = *(uint16_t*)&MMNL_inbuf[0]; // header reader
		uint16_t s_adr = *(uint16_t*)&MMNL_inbuf[2];
		uint8_t f = MMNL_inbuf[4];
		uint8_t ind = MMNL_inbuf[5];
		uint8_t l = MMNL_inbuf[6];
		uint8_t* data = MMNL_inbuf + 7;
		if (f & DIRECT_RETRANSLATION_FLAG)
			data++;
		if (f & MODULE_RETRANSLATION_FLAG)
			data++;
		if (f & NETWORK_RETRANSLATION_FLAG)
			data++;

		if (data - MMNL_inbuf + l != MMNL_inbuf_size || MMNL_inbuf_size >= 256) // check size
			return;


		if (f & SYSTEM_FLAG) { // check system flags round 1
			uint8_t sys_id = data[0];
			data++;
			switch (sys_id) {
			case MMNL_FIND_NET:
				if (f & ANSVER_FLAG) {
					if (local_ard == 0 && memcmp(data + 2, &rnd_v, 4) == 0) {
						memcpy(&local_ard, data, 2);
						state = WaitForAddressVerification;
						mmnl_tim = MMNL_MILLIS();
						MMNL_printf("New addr %d in %d net\n", MMNL_GET_ADR(local_ard), MMNL_GET_NETWORK(local_ard));
						uint8_t buf[5];
						buf[0] = MMNL_NEW_DEV;
						memcpy(buf + 1, &rnd_v, 4);
						make_packet(MMNL_GET_BROADCAST(local_ard), SYSTEM_FLAG | DIRECT_RETRANSLATION_FLAG | MODULE_RETRANSLATION_FLAG, mmnl_i++, buf, 5, 4, 4);
						send_buf();
						dv.load_structure(data + 6);
						dv.new_dev(MMNL_GET_ADR(local_ard));
					}
				}
				else {
					if (local_ard != 0) {
						uint16_t m = dv.get_next_adr() | MMNL_GET_BROADCAST(local_ard);
						uint8_t t_buf[15]; // id + new adr + rand + dv
						t_buf[0] = sys_id;
						memcpy(t_buf + 1, &m, 2);
						memcpy(t_buf + 3, data, 4);
						dv.save_structure(t_buf + 7);
						make_packet(s_adr, f | ANSVER_FLAG, mmnl_i++, t_buf, 15);
						send_buf();
					}
					else {
						if (ind >= mmnl_i)
							mmnl_tim += rand() % 100;
					}

				}
				break;
			case MMNL_NEW_DEV:
			{
				if (MMNL_GET_NETWORK(s_adr) != MMNL_GET_NETWORK(local_ard))
					break;

				if (s_adr == local_ard && memcmp(data, &rnd_v, 4) != 0) {
					uint32_t r_rnd_v = 0;
					memcpy(&r_rnd_v, data, 4);
					if (state != WaitForAddressVerification || state == WaitForAddressVerification && rnd_v < r_rnd_v) {
						uint8_t buf[1];
						buf[0] = MMNL_NEW_DEV;
						make_packet(local_ard, SYSTEM_FLAG | DIRECT_RETRANSLATION_FLAG | MODULE_RETRANSLATION_FLAG, ind++, buf, 1, 4, 4);
						send_buf();
						return;
					}
					else {
						state = Init;
						return;
					}
				}

				uint16_t adr = MMNL_GET_ADR(s_adr);
				dv.new_dev(adr);
				piw.is_good(adr, ind - 1, true);
			}
			break;
			case MMNL_KEEP_ADR:
				if(state == WaitForAddressVerification && d_adr == local_ard) {
					state = Init;
					return;
				}
				break;
			}
		}

		if (s_adr == local_ard)
			return;

		if (MMNL_GET_NETWORK(s_adr) != MMNL_GET_NETWORK(local_ard)
			&& MMNL_GET_NETWORK(d_adr) != MMNL_GET_NETWORK(local_ard))
			if (f & NETWORK_RETRANSLATION_FLAG) {
				uint8_t& nttl = data[-1];
				if (nttl)
					--nttl;
				else
					return;
			}
			else
				return;


		if (s_adr != 0 && !piw.is_good(s_adr, ind))
			return;

		if (f & SYSTEM_FLAG) {
			uint8_t sys_id = data[0];
			data++;
			switch (sys_id) {
			case MMNL_PING:
				if (!(f & ANSVER_FLAG))
					make_packet(s_adr, f | ANSVER_FLAG, mmnl_i++, data, l);
				break;
			case MMNL_FIND_NAME:
				if (!(f & ANSVER_FLAG)) {
					if (data[l] == 0 && strcmp((const char*)data, dev_name) == 0)
						make_packet(s_adr, f | ANSVER_FLAG, mmnl_i++, dev_name, strlen(dev_name) + 1);
				}
				break;
			}
		}


		bool it_for_me = d_adr == local_ard;
		bool it_for_all = d_adr == MMNL_GET_BROADCAST(local_ard);

		if (!it_for_me && (f & (DIRECT_RETRANSLATION_FLAG | MODULE_RETRANSLATION_FLAG))) {
			uint8_t bp = 7;
			if (f & DIRECT_RETRANSLATION_FLAG) {
				uint8_t &dttl = MMNL_inbuf[bp++];

				if(dttl) {
					--dttl;
					mlist[cur_ind]->send(MMNL_inbuf, MMNL_inbuf_size);
					++dttl;
				}
			}
			
			if (f & MODULE_RETRANSLATION_FLAG) {
				uint8_t &mttl = MMNL_inbuf[bp++];

				if (mttl) {
					--mttl;
					for (int i = 0; i < mlist_size; ++i)
						if (i != cur_ind)
							mlist[i]->send(MMNL_inbuf, MMNL_inbuf_size);
					++mttl;
				}
			}
		}

		if ((f & SYSTEM_FLAG) == 0 && (it_for_me || it_for_all)) {
			user_packet_callback(s_adr, data, l, it_for_all);
		}
	}

	void MMNL::send(uint16_t adr, void* buf, int l, uint8_t dttl, uint8_t mttl, uint8_t nttl) {
		uint8_t f = 0;
		if (dttl)
			f |= DIRECT_RETRANSLATION_FLAG;
		if (mttl)
			f |= MODULE_RETRANSLATION_FLAG;
		if (nttl)
			f |= NETWORK_RETRANSLATION_FLAG;
		make_packet(adr, f, mmnl_i++, buf, l, dttl, mttl, nttl);
		send_buf();
	}

	void MMNL::update() {
		for (int i = 0; i < mlist_size; ++i) {
			cur_ind = i;
			int a = 0;
			mlist[i]->update(MMNL_inbuf, MMNL_inbuf_size, this, &MMNL::pcocess_packet);
		}

		switch (state)
		{
		case Init:
			mmnl_i = 0;
			uint8_t buf[5];
			buf[0] = MMNL_FIND_NET;
			memcpy(buf + 1, &rnd_v, 4);
			make_packet(MMNL_GET_BROADCAST(local_ard), SYSTEM_FLAG, mmnl_i++, buf, 5, 4, 4);
			send_buf();
			state = WaitNetwork;
			mmnl_tim = MMNL_MILLIS();
			break;

		case WaitNetwork:
			if (MMNL_MILLIS() - mmnl_tim > 1000) {
				mmnl_tim = MMNL_MILLIS();
				if (mmnl_i == 5) {
					state = Idle;
					int net_id = (rand() % 31) + 1;
					local_ard = (net_id << (16 - 5)) | 1;
					MMNL_printf("New addr %d in %d net\n", MMNL_GET_ADR(local_ard), MMNL_GET_NETWORK(local_ard));
					dv.init(MMNL_GET_ADR(local_ard));
				}
				else {
					uint8_t buf[5];
					buf[0] = MMNL_FIND_NET;
					memcpy(buf + 1, &rnd_v, 4);
					make_packet(MMNL_GET_BROADCAST(local_ard), SYSTEM_FLAG, mmnl_i++, buf, 5, 4, 4);
					send_buf();
				}
			}
			break;

		case WaitForAddressVerification:
			if (MMNL_MILLIS() - mmnl_tim > 1000) {
				state = Idle;
				mmnl_tim = MMNL_MILLIS();
			}
			break;

		case Idle:
			break;

		default:
			break;
		}
	}

	uint16_t MMNL::get_my_address() {
		return local_ard;
	}

	State MMNL::get_state()
	{
		return state;
	}

	void MMNL::make_packet(uint16_t adr, uint8_t f, uint8_t i, const void* buf, int l, uint8_t dttl, uint8_t mttl, uint8_t nttl) {
		*(uint16_t*)&MMNL_buf[0] = adr;
		*(uint16_t*)&MMNL_buf[2] = local_ard;
		MMNL_buf[4] = f;
		MMNL_buf[5] = i;
		MMNL_buf[6] = l;
		uint8_t c = 7;
		if (f & DIRECT_RETRANSLATION_FLAG)
			MMNL_buf[c++] = dttl;
		if (f & MODULE_RETRANSLATION_FLAG)
			MMNL_buf[c++] = mttl;
		if (f & NETWORK_RETRANSLATION_FLAG)
			MMNL_buf[c++] = nttl;
		//if (f & FRAGMENTATION_FLAG)
		//	MMNL_buf[c++] = fid;
		memcpy(MMNL_buf + c, buf, l);
		MMNL_buf_size = c + l;
	}
};


