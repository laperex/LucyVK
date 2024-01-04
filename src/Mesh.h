#pragma once

#include "lucyvk/lucyvk.h"
#include <vector>
#include <glm/glm.hpp>
// #include <math/math.hpp>

namespace lucy {
	struct MeshPushConstants {
		glm::vec4 offset;
		glm::mat4 render_matrix;
	};

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;

		static auto get_vertex_description() -> lvk::vertex_input_description<1, 3> {
			return lvk::vertex_input_description {
				{
					{
						.binding = 0,
						.stride = sizeof(Vertex),
						.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
					}
				},
				{
					{
						.location = 0,
						.binding = 0,
						.format = VK_FORMAT_R32G32B32_SFLOAT,
						.offset = offsetof(lucy::Vertex, position),
					},
					{
						.location = 1,
						.binding = 0,
						.format = VK_FORMAT_R32G32B32_SFLOAT,
						.offset = offsetof(lucy::Vertex, normal),
					},
					{
						.location = 2,
						.binding = 0,
						.format = VK_FORMAT_R32G32B32_SFLOAT,
						.offset = offsetof(lucy::Vertex, color),
					}
				}
			};
		}
	};

	struct Mesh {
		std::vector<Vertex> _vertices;

		lvk_buffer vertex_buffer;
		
		bool load_obj(const char* filename);
	};
}
