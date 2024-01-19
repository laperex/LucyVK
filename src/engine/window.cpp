#include "engine/window.h"
#include <stdexcept>

void lucy::window::initialize(const char* name, const glm::ivec2 offset, const glm::ivec2 size) {
	window = SDL_CreateWindow(name, offset.x, offset.y, size.x, size.y, SDL_WINDOW_VULKAN);

	if (window == nullptr) {
		throw std::runtime_error("sdl window creation failed");
	}
}

glm::ivec2 lucy::window::size() {
	glm::ivec2 s;
	SDL_GetWindowSize(window, &s.x, &s.y);
	return s;
}
