#pragma once

#include "engine/events.h"
#include "engine/renderer.h"
#include "engine/window.h"

namespace lucy {
	class engine {
		lucy::window _window;
		lucy::renderer _renderer; 
		lucy::events _events;
	
	public:
		void initialize();
		void mainloop();

		void destroy();
	};
}