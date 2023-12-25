#pragma once

#include "lucyvk/vk_static.h"
#include <vector>
#include <glm/vec3.hpp>

namespace lucy {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
	};

	struct Mesh {
		std::vector<Vertex> _vertices;

		lvk_buffer _vertex_buffer;
	};
}
