#pragma once

#include "lucy/events.h"
#include "lucy/window.h"
#include "lucyvk/renderer.h"
	
namespace lucy {
	class engine;

	class system_template {
	protected:
		lucy::window* _window;
		lucy::renderer* _renderer;
		lucy::events* _events;

	public:
		virtual void initialization() = 0;
		virtual void update(double dt) = 0;
		virtual void destroy() = 0;
		
		friend engine;
	};
}