#pragma once

#include <math/math.hpp>
#include <SDL_video.h>

namespace lucy {
	struct window {
		SDL_Window* window;
		
		void initialize(const char* name, const glm::ivec2 offset, const glm::ivec2 size);
		
		glm::ivec2 size();
	};
}