#include "engine.h"
#include "lucyre/renderer.h"
#include "lucy/events.h"
#include "lucy/window.h"

#include <chrono>

static lucy::engine _engine;

lucy::engine::engine() {
	registry.add<lre::renderer>();
	registry.add<lucy::window>();
	registry.add<lucy::events>();
}

void lucy::engine::initialize() {
	auto& _events = _engine.registry.get<lucy::events>();
	auto& _window = _engine.registry.get<lucy::window>();
	auto& _renderer = _engine.registry.get<lre::renderer>();

	// _window.initialize("lucy", { 50, 50 }, { 1920, 1080 });
	_window.initialize("lucy", { 50, 50 }, { 1920, 1080 });

	_renderer.initialization(_window.window);
}


void lucy::engine::mainloop() {
	auto& _window = _engine.registry.get<lucy::window>();
	auto& _events = _engine.registry.get<lucy::events>();
	auto& _renderer = _engine.registry.get<lre::renderer>();

	double dt = 0;
	while (!_events.quit()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();
		
		_events.update();
		
		if (_events.key_pressed(SDL_SCANCODE_ESCAPE)) {
			_events.quit() = true;
		}

		_renderer.update();

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
}

void lucy::engine::destroy() {
	auto& _renderer = _engine.registry.get<lre::renderer>();

	_renderer.destroy();
}

ltl::registry& lucy::engine::get_registry() {
	return _engine.registry;
}
