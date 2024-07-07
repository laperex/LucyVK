#pragma once

#include "lucylm/math.hpp"
#include "lucyvk/create_info.h"
#include "lucyvk/device.h"
// #include "lucyvk/memory.h"


namespace lvk {
	struct vertex_input_description {
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};


	struct vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 uv;


		static vertex_input_description get_vertex_input_description() {
			return {
				.bindings = {
					lvk::info::vertex_input_description<vertex>(0, VK_VERTEX_INPUT_RATE_VERTEX)
				},

				.attributes = {
					lvk::info::vertex_input_attribute_description(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, position)),
					lvk::info::vertex_input_attribute_description(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, normal)),
					lvk::info::vertex_input_attribute_description(0, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(vertex, uv)),
				},

				.flags = 0
			};
		};
	};


	struct mesh {
		std::vector<vertex> vertices;
		std::vector<uint32_t> indices;

		lvk_buffer vertex_buffer;
		lvk_buffer index_buffer;
		
		void load_obj(const char* filename);
	};
}

