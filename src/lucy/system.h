#pragma once

#include "lucy/events.h"
#include "lucy/window.h"
#include "lucyre/renderer.h"
	
namespace lucy {
	class system_template {
	protected:
		lucy::window& _window;
		lre::renderer& _renderer;
		lucy::events& _events;

	public:
		system_template(
			lucy::window &window,
			lre::renderer &renderer,
			lucy::events &events) :
			_window(window),
			_renderer(renderer),
			_events(events) {}

		virtual void initialization() = 0;
		virtual void update() = 0;
		virtual void destroy() = 0;
	};
}