#include "Module.h"

#include <SFML/Network.hpp>

namespace MMNL {
	namespace Module {
		const uint16_t MMNL_port = 17788;

		class UDP_SFML : Base {
			sf::UdpSocket sock;

		public:
			sf::IpAddress last_ip;

			bool init() override {
				MMNL_printf("[UDP] Initializing ... ");
				sf::Socket::Status state = sock.bind(MMNL_port);
				if (state != sf::Socket::Status::Done) {
					MMNL_printf("failed, code %d\n", state);
					return true;
				}
				sock.setBlocking(0);
				MMNL_printf("success!\n");
				this->state = Receiving;

				return false;
			}

			void send(uint8_t* buf, uint16_t len) override {
				sock.send(buf, len, sf::IpAddress::Broadcast, MMNL_port);
			}

			void update(uint8_t* buf, uint16_t& len, MMNL* b, void (MMNL::*handle)()) override {
				uint8_t temp_buf[65535];
				size_t recived = 0;
				sf::IpAddress ip;
				uint16_t port;
				sf::Socket::Status state = sock.receive(temp_buf, 65535, recived, ip, port);

				if (ip.getLocalAddress() == ip)
					return;

				if(ip.toInteger()!=0)
					last_ip = ip;

				if (state == sf::Socket::Status::Done) {
					MMNL_printf("[UDP] New pack %d (from %s)\n", (int)recived, ip.toString().c_str());
					for (int i = 0; i < recived; ++i)
						MMNL_printf("%02X ", temp_buf[i]);
					MMNL_printf("\n");

					if (recived > 256) {
						MMNL_printf("[UDP] to long %d\n", (int)recived);
						return;
					}
					memcpy(buf, temp_buf, recived);
					len = recived;
					(b->*handle)();
				}
			}

			uint16_t get_max_size() override {
				return 256;
			}
		};
	};
};
