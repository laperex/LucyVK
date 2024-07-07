#pragma once

#include "system.h"
#include <memory>

namespace lucy {
	class engine {
		std::vector<std::shared_ptr<system_template>> system_array;

		lucy::renderer _renderer;
		lucy::window _window;
		lucy::events _events;

	public:
		template <typename T>
		void add() {
			system_array.push_back(std::static_pointer_cast<system_template>(std::make_shared<T>()));
			system_array.back()->_events = &_events;
			system_array.back()->_renderer = &_renderer;
			system_array.back()->_window = &_window;
		}
		void initialize();
		void mainloop();
		void destroy();
	};
}