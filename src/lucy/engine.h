#pragma once

#include "lucytl/registry.h"

namespace lucy {
	template <typename T> T& static_store() { static T v = { }; return v; }

	namespace engine {
		void initialize();

		void mainloop();

		void destroy();
	};
}