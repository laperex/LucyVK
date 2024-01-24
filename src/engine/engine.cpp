#include "engine.h"

#include <chrono>



void lucy::engine::initialize(std::function<void()> function) {
	_window.initialize("lucy", { 50, 50 }, { 1920, 1080 });

	_renderer.initialization(&_window);
	
	function();
}


void lucy::engine::mainloop(std::function<void(double)> function) {
	double dt = 0;
	while (!_events.quit()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();
		
		_events.update();
		
		function(dt);
		
		if (_events.key_pressed(SDL_SCANCODE_ESCAPE)) {
			_events.quit() = true;
		}

		_renderer.update();

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
}

void lucy::engine::destroy() {
	_renderer.destroy();
}
