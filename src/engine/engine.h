#pragma once

#include "engine/events.h"
#include "engine/renderer.h"
#include "engine/window.h"

namespace lucy {
	struct engine {
		lucy::window _window;
		lucy::renderer _renderer; 
		lucy::events _events;

		void initialize(std::function<void()>);
		void mainloop(std::function<void(double)>);

		void destroy();
	};
}