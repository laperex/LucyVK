#pragma once

#include "lvm_vector.h"

namespace lucy {
	glm::mat4 projection_orthographic();
	glm::mat4 projection_perspective(uint32_t width, uint32_t height, float fov, float near, float far);

	glm::mat4 view_pan_around_point(glm::vec3 rotation);
	glm::mat4 view_at_point();
}
