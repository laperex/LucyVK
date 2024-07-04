#pragma once

#include "lucylm/math.hpp"
#include "lucyvk/device.h"
// #include "lucyvk/memory.h"


namespace lre {
	struct VertexInputDescription {
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};


	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 uv;


		static VertexInputDescription get_vertex_input_description() {
			return {
				.bindings = {
					VkVertexInputBindingDescription {
						.binding = 0,
						.stride = sizeof(Vertex),
						.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
					}
				},

				.attributes = {
					VkVertexInputAttributeDescription {
						.location = 0,
						.binding = 0,
						.format = VK_FORMAT_R32G32B32_SFLOAT,
						.offset = offsetof(Vertex, position),
					},
					VkVertexInputAttributeDescription {
						.location = 1,
						.binding = 0,
						.format = VK_FORMAT_R32G32B32_SFLOAT,
						.offset = offsetof(Vertex, normal),
					},
					VkVertexInputAttributeDescription {
						.location = 2,
						.binding = 0,
						.format = VK_FORMAT_R32G32B32_SFLOAT,
						.offset = offsetof(Vertex, uv),
					},
				},

				.flags = 0
			};
		};
	};


	struct Mesh {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		lvk_buffer vertex_buffer;
		lvk_buffer index_buffer;
		
		void load_obj(const char* filename);
	};
}

