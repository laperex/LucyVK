#pragma once

#include "lucytl/registry.h"

namespace lucy {
	struct engine {
		ltl::registry registry;
		
		engine();
	
		static void initialize();
		static void mainloop();

		static void destroy();
		
		static ltl::registry& get_registry();
	};
}