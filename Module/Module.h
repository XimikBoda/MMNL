#pragma once
#include "../MMNL_Conf.h"
#include <stdlib.h>

namespace MMNL {
	class MMNL;

	namespace Module {
		enum State : uint8_t {
			Idle,
			Receiving,
			Sending,
			Error,
		};

		class Base {
		protected:
			State state = Idle;
		public:
			virtual bool init() { return true; };
			virtual void send(uint8_t* buf, uint16_t len) {};
			virtual void update(uint8_t* buf, uint16_t& len, MMNL* b, void (MMNL::* handle)()) {};
			virtual uint16_t get_max_size() { return 0; };
			State get_state() { return state; };
		};
	};
};