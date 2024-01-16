#pragma once

#include "vk_renderer.h"
#include "sdl_window.h"

namespace lucy {
	class engine {
		sdl_window window;
		vk_renderer renderer; 
	
	public:
		void initialize();
		void mainloop();

		void destroy();
	};
}