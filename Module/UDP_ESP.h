#include "Module.h"

#include <WiFiUdp.h>


namespace MMNL {
	namespace Module {
		const uint16_t MMNL_port = 17788;

		class UDP_ESP : Base {
			WiFiUDP sock;

			bool init() override {
				MMNL_printf("[UDP] Initializing ... ");
				sock.begin(MMNL_port);
				MMNL_printf("success!\n");

				this->state = Receiving;

				return false;
			}

			void send(uint8_t* buf, uint16_t len) override {
				sock.beginPacket(IPAddress(0xFFFFFFFF), MMNL_port);
				sock.write(buf, len);
				sock.endPacket();
			}

			void update(uint8_t* buf, uint16_t& len, MMNL* b, void (MMNL::* handle)()) override {
				size_t recived = sock.parsePacket();

				if (recived > 0) {
					if (recived > 256) {
						MMNL_printf("[UDP] to long %d\n", (int)recived);
						return;
					}
					sock.read(buf, recived);

					len = recived;
					MMNL_printf("[UDP] New pack %d\n", (int)len);
					for (int i = 0; i < len; ++i)
						MMNL_printf("%02X ", buf[i]);
					MMNL_printf("\n");

					(b->*handle)();
				}
			}

			uint16_t get_max_size() override {
				return 256;
			}
		};
	};
};
