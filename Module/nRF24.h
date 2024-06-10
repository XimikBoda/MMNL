#pragma once
#include "Module.h"

#include <RadioLib.h>

namespace MMNL {
	namespace Module {
		class nRF24 : Base {
			::nRF24 nrf24;

		public:
			nRF24(::nRF24 nrf24_) : nrf24(nrf24_) {}

			bool init() override;
			void send(uint8_t* buf, uint16_t len) override;
			void update(uint8_t* buf, uint16_t& len, MMNL* b, void (MMNL::* handle)()) override;
			uint16_t get_max_size() override;

		private:
			void start_lisening();
		};
	};
};


#ifdef MMNL_MODULE_IMPLEMENTATION

namespace MMNL {
	namespace Module {
		volatile static uint8_t nrf24_received_flag = 0; // 0 - done, 1 - flag, 2 - procces

		MMNL_ISR static nrf24_set_received_flag() {
			nrf24_received_flag = 1;
		}

		bool nRF24::init() {
			MMNL_printf("[nRF24] Initializing ... ");
			int state = nrf24.begin(2400 + 0x7A, 2000);
			if (state != RADIOLIB_ERR_NONE) {
				MMNL_printf("failed, code %d\n", state);
				return true;
			}
			MMNL_printf("success!\n");

			unsigned char addr[] = { '1', 'M', 'M', 'N', 'L' };
			MMNL_printf("[nRF24] Setting transmit pipe ... ");
			state = nrf24.setTransmitPipe(addr);
			if (state == RADIOLIB_ERR_NONE) {
				MMNL_printf("success!\n");
			}
			else {
				MMNL_printf("failed, code %d\n", state);
				return true;
			}

			nrf24.setAutoAck(false);

			return false;
		}

		void nRF24::start_lisening() {
			nrf24_received_flag = 2;
			nrf24.setPacketReceivedAction(nrf24_set_received_flag);
			nrf24.startReceive();
			state = Receiving;
		}

		void nRF24::send(uint8_t* buf, uint16_t len) {
			nrf24.transmit(buf, (size_t)len, 0);
			start_lisening();
		}

		void nRF24::update(uint8_t* buf, uint16_t& len, MMNL* b, void (MMNL::* handle)()) {
			if (nrf24_received_flag == 1) {
				len = nrf24.getPacketLength();
				int state = nrf24.readData(buf, len);
				state = Idle;
				nrf24_received_flag = 0;
				MMNL_printf("[nRF24] New pack %d\n", len);
				for (int i = 0; i < len; ++i)
					MMNL_printf("%02X ", buf[i]);
				MMNL_printf("\n");
				//printf("%s\n", MMNL_inbuf);

				if (state == RADIOLIB_ERR_NONE) {
					(b->*handle)();
				}
				else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
					MMNL_printf("[nRF24] CRC error!\n");
				}
				else {
					MMNL_printf("[nRF24] Failed, code %d\n", state);
				}
				MMNL_printf("RSSI: %f dBm, ", nrf24.getRSSI());
				MMNL_printf("SNR: %f dB\n", nrf24.getSNR());
			}
			if (state == Idle)
				start_lisening();

		}

		uint16_t nRF24::get_max_size() {
			return 32;
		}
	};
};

#endif