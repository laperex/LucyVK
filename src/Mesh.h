#pragma once

#include "lucyvk/vk_static.h"
#include <vector>
#include <glm/glm.hpp>
// #include <math/math.hpp>

namespace lucy {
	struct MeshPushConstants {
		glm::vec4 data;
		glm::mat4 render_matrix;
	};

	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		
		static lvk::vertex_input_description get_vertex_description() {
			lvk::vertex_input_description description;

			//we will have just 1 vertex buffer binding, with a per-vertex rate
			VkVertexInputBindingDescription mainBinding = {};
			mainBinding.binding = 0;
			mainBinding.stride = sizeof(Vertex);
			mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

			description.bindings.push_back(mainBinding);

			//Position will be stored at Location 0
			VkVertexInputAttributeDescription positionAttribute = {};
			positionAttribute.binding = 0;
			positionAttribute.location = 0;
			positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			positionAttribute.offset = offsetof(Vertex, position);

			//Normal will be stored at Location 1
			VkVertexInputAttributeDescription normalAttribute = {};
			normalAttribute.binding = 0;
			normalAttribute.location = 1;
			normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			normalAttribute.offset = offsetof(Vertex, normal);

			//Color will be stored at Location 2
			VkVertexInputAttributeDescription colorAttribute = {};
			colorAttribute.binding = 0;
			colorAttribute.location = 2;
			colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
			colorAttribute.offset = offsetof(Vertex, color);

			description.attributes.push_back(positionAttribute);
			description.attributes.push_back(normalAttribute);
			description.attributes.push_back(colorAttribute);

			return description;
		}
	};

	struct Mesh {
		std::vector<Vertex> _vertices;

		lvk_buffer vertex_buffer;
		
		bool load_obj(const char* filename);
	};
}
