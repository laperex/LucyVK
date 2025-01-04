// #include "lucy/engine.h"
#include "lucyio/logger.h"
#include <lucy/engine.h>
#include <stdio.h>
#include <unistd.h>
#include <vulkan/vulkan_core.h>

#include "lucytl/linked_list.h"
#include "lucytl/static_list.h"
#include "lucytl/array.h"
#include "voxel/camera.h"
#include "voxel/world.h"


// // TODO: dynamic viewport
// TODO: renderer restructuring
	// // TODO: semaphore unsignaling
	//! TODO: mesh loading
	// TODO: material management
	// TODO: shader management
// // TODO: memory handles
//! TODO: render_pass restructuring
//! TODO: better memory management
// // TODO: Implement Better Destruction -
// TODO: Implement Image Loading
//! TODO: Implement DearImGui

void a() {
	printf("sdkjansd");
}

typedef void(*NULL_DU)();

int main(int count, char** args) {
	lucy::engine engine;

	engine.add<lucy::camera>();
	engine.add<lucy::world>();

	engine.initialize();
	engine.mainloop();
	engine.destroy();
}


	// lvk_pipeline graphics_pipeline = {};
	// {
	// 	lvk::config::graphics_pipeline config = {
	// 		.shader_stage_array = {
	// 			lvk::info::shader_stage(&vertex_shader, nullptr),
	// 			lvk::info::shader_stage(&fragment_shader, nullptr),
	// 		},

	// 		.vertex_input_state = lvk::info::vertex_input_state(
	// 			{
	// 				{
	// 					.binding = 0,
	// 					.stride = sizeof(lucy::Vertex),
	// 					.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	// 				}
	// 			},
	// 			{
	// 				{
	// 					.location = 0,
	// 					.binding = 0,
	// 					.format = VK_FORMAT_R32G32B32_SFLOAT,
	// 					.offset = offsetof(lucy::Vertex, position),
	// 				},
	// 				{
	// 					.location = 1,
	// 					.binding = 0,
	// 					.format = VK_FORMAT_R32G32B32_SFLOAT,
	// 					.offset = offsetof(lucy::Vertex, normal),
	// 				},
	// 				{
	// 					.location = 2,
	// 					.binding = 0,
	// 					.format = VK_FORMAT_R32G32B32_SFLOAT,
	// 					.offset = offsetof(lucy::Vertex, color),
	// 				}
	// 			}
	// 		),
	// 		.input_assembly_state = lvk::info::input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false),
	// 		.rasterization_state = lvk::info::rasterization_state(VK_POLYGON_MODE_FILL),
	// 		.multisample_state = lvk::info::multisample_state(),
	// 		.depth_stencil_state = lvk::info::depth_stencil_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL),
	// 		.color_blend_attachment = lvk::color_blend_attachment(),
			
	// 		.viewport = {
	// 			.x = 0.0f,
	// 			.y = 0.0f,
	// 			.width = (float)window.size.x,
	// 			.height = (float)window.size.y,
	// 			.minDepth = 0.0f,
	// 			.maxDepth = 1.0f
	// 		},

	// 		.scissor = {
	// 			.offset = { 0, 0 },
	// 			.extent = { static_cast<uint32_t>(window.size.x), static_cast<uint32_t>(window.size.y) }
	// 		}
	// 	};

	// 	graphics_pipeline = pipeline_layout.init_graphics_pipeline(&render_pass, &config);
	// }