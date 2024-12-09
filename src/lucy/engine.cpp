#include <chrono>

#include "engine.h"
#include "lucy/events.h"
#include "lucy/window.h"
#include "lucyvk/renderer.h"
#include "voxel/camera.h"

template <typename T> T& static_store() { static T v = { }; return v; }

void lucy::engine::initialize() {
	// auto& _renderer = static_store<lucy::renderer>();
	// auto& _window = static_store<lucy::window>();
	// auto& _events = static_store<lucy::events>();
	
	_window.initialize("lucy", { 50, 50 }, { 1920, 1080 });
	_renderer.init(_window);
	
	// lucy::camera cam = { _window, _renderer, _events };
	
	for (auto system: system_array) {
		system->initialization();
	}
}

void lucy::engine::mainloop() {
	// auto& _renderer = static_store<lucy::renderer>();
	// auto& _window = static_store<lucy::window>();
	// auto& _events = static_store<lucy::events>();

	double dt = 0;
	while (!_events.quit()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		_events.update();

		if (_events.key_pressed(SDL_SCANCODE_ESCAPE)) {
			_events.quit() = true;
		}

		for (auto system: system_array) {
			system->update(dt);
		}

		_renderer.update(_events.resized());

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
}

void lucy::engine::destroy() {
	_renderer.destroy();
}

