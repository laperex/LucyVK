#pragma once

#include <math/math.hpp>

namespace lucy {
	struct camera {
		glm::mat4 projection;
		glm::mat4 view;
		
		glm::vec3 position = { 0.0, 0.0, 0.0 };
		
		glm::vec3 world_front = { 0.0, 0.0, 1.0 };
		glm::vec3 front = world_front;
		glm::vec3 world_up = { 0.0, -1.0, 0.0 };
		glm::vec3 up = world_up;
	};
}