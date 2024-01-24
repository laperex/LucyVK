#include "engine.h"
#include "Events.h"

#include <chrono>

void lucy::engine::initialize() {
	_window.initialize("lucy", { 50, 50 }, { 1920, 1080 });

	_renderer.initialization(&_window);
}

void lucy::engine::mainloop() {
	double dt = 0;
	while (!_events.quit()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		_events.update();

		_renderer.update();
		
		if (_events.key_pressed(SDL_SCANCODE_ESCAPE)) {
			_events.quit() = true;
		}

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
}

void lucy::engine::destroy() {
	
}
