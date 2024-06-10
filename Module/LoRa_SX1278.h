#pragma once
#include "Module.h"

#include <RadioLib.h>

namespace MMNL {
	namespace Module {
		class LoRa_SX1278 : Base {
			SX1278 lora;

		public:
			LoRa_SX1278(SX1278 lora_) : lora(lora_) {}

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
        volatile static uint8_t sx1278_transmitted_flag = 0; // 0 - done, 1 - flag, 2 - procces
        volatile static uint8_t sx1278_received_flag = 0; // 0 - done, 1 - flag, 2 - procces

        MMNL_ISR static sx1278_set_transmitted_flag() {
            sx1278_transmitted_flag = 1;
        }

        MMNL_ISR static sx1278_set_received_flag() {
            sx1278_received_flag = 1;
        }

        bool LoRa_SX1278::init() {
            MMNL_printf("[SX1278] Initializing ... ");
            int state = lora.begin(433.875, 62.5, 7, 5, 'M', 13);
            if (state != RADIOLIB_ERR_NONE) {
                MMNL_printf("failed, code %d\n", state);
                return true;
            }
            MMNL_printf("success!\n");
            return false;
        }

        void LoRa_SX1278::send(uint8_t* buf, uint16_t len) {
            lora.standby();
            lora.setPacketSentAction(sx1278_set_transmitted_flag);
            lora.startTransmit(buf, len);
            sx1278_transmitted_flag = 2;
            state = Sending;
        }

        void LoRa_SX1278::update(uint8_t* buf, uint16_t& len, MMNL* b, void (MMNL::* handle)()) {
            if (sx1278_transmitted_flag == 1) {
                sx1278_transmitted_flag = 0;
                lora.finishTransmit();
                state = Idle;
            }
            if (sx1278_received_flag == 1) {
                len = lora.getPacketLength();
                int state = lora.readData(buf, len);
                state = Idle;
                sx1278_received_flag = 0;
                MMNL_printf("[SX1278] New pack %d\n", len);
                for (int i = 0; i < len; ++i)
                    MMNL_printf("%02X ", buf[i]);
                MMNL_printf("\n");

                if (state == RADIOLIB_ERR_NONE) {
                    (b->*handle)();
                }
                else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
                    MMNL_printf("[SX1278] CRC error!\n");
                }
                else {
                    MMNL_printf("[SX1278] Failed, code %d\n", state);
                }
                MMNL_printf("RSSI: %f dBm, ", lora.getRSSI());
                MMNL_printf("SNR: %f dB, ", lora.getSNR());
                MMNL_printf("Frequency error: %f Hz\n", lora.getFrequencyError());
            }
            if (state == Idle) {
                lora.setPacketReceivedAction(sx1278_set_received_flag);
                lora.startReceive();
                state = Receiving;
                sx1278_received_flag = 2;
            }
        }

		uint16_t LoRa_SX1278::get_max_size() {
			return 256;
		}
	};
};

#endif