// #include "lucy/engine.h"
#include <chrono>
#include "lucyio/logger.h"
#include "lucyre/renderer.h"
#include "lucy/events.h"
#include "lucy/window.h"
#include <stb_image.h>

#include <vulkan/vulkan.hpp>


template <typename T>
T& static_store() {
	static T v = { };
	return v;
}


int main(int count, char** args) {
	lucy::events _events;
	lucy::window _window;
	auto& _renderer = static_store<lre::renderer>();
	// d.
	// a.
	// d.waitForFences()
	// vk::ArrayProxy<uint32_t> a = { 0, 1, 2 };
	// d.createFence()
	// VkFenceCreateInfo a;
	// d.createFenceUnique()
	
	_window.initialize("lucy", { 50, 50 }, { 1920, 1080 });

	_renderer.initialization(_window.window);
	
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
	
	_renderer.destroy();
}