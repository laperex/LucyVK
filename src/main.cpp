
#include "lucyvk/vk_instance.h"
#include "lucyvk/vk_physical_device.h"
#include "lucyvk/vk_device.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Camera.h"
#include "Mesh.h"
#include "lucyvk/vk_config.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_info.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <Events.h>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>
#include <Window.h>
#include <glm/glm.hpp>

#include <glm/gtx/transform.hpp>

#include <util/logger.h>

typedef uint32_t lve_vertex;

#define LVE_CHUNK_SIZE (32)

#define LVE_VERTEX(x, y, z, n, u, v) ((x * LVE_CHUNK_SIZE * LVE_CHUNK_SIZE) + y * LVE_CHUNK_SIZE + z) & 0x7fff

// static lvk::vertex_input_description lve_vertex_description() {
// 	lvk::vertex_input_description description;

// 	//we will have just 1 vertex buffer binding, with a per-vertex rate
// 	VkVertexInputBindingDescription mainBinding = {};
// 	mainBinding.binding = 0;
// 	mainBinding.stride = sizeof(lve_vertex);
// 	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

// 	description.bindings.push_back(mainBinding);

// 	//Position will be stored at Location 0
// 	VkVertexInputAttributeDescription positionAttribute = {};
// 	positionAttribute.binding = 0;
// 	positionAttribute.location = 0;
// 	positionAttribute.format = VK_FORMAT_R32_UINT;
// 	positionAttribute.offset = 0;

// 	description.attributes.push_back(positionAttribute);

// 	return description;
// }

std::vector<lve_vertex> lve_vertex_triangle = {
	LVE_VERTEX(0, 0, 0, 1, 0, 0),
	LVE_VERTEX(0, 0, 0, 1, 1, 1),
};

lucy::Mesh load_triangle_mesh() {
	lucy::Mesh triangle_mesh;
	
	triangle_mesh._vertices.resize(3);

	//vertex positions
	triangle_mesh._vertices[0].position = { 1.f, 1.f, 0.0f };
	triangle_mesh._vertices[1].position = {-1.f, 1.f, 0.0f };
	triangle_mesh._vertices[2].position = { 0.f,-1.f, 0.0f };

	//vertex colors, all green
	triangle_mesh._vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	triangle_mesh._vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	triangle_mesh._vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green
	
	return triangle_mesh;
}

struct mvp_matrix {
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
};

struct Frame {
	lvk_fence render_fence;
	lvk_semaphore present_semaphore;
	lvk_semaphore render_semaphore;

	lvk_command_pool command_pool;
	lvk_command_buffer command_buffer;
	
	uint32_t image_index;
	
	lvk_buffer camera_buffer;

	lvk_descriptor_set global_descriptor;
};

static constexpr const int FRAMES_IN_FLIGHT = 2;

int main(int count, char** args) {
	lucy::Window window = {};
	window.InitWindow();

	lvk::config::instance instance_config = {
		.name = "Lucy Framework v7",
		.enable_validation_layers = true
	};
	
	//* ---------------> INSTANCE INIT

	auto instance = lvk_init_instance(&instance_config, window.sdl_window);

	auto physical_device = instance.init_physical_device();
	auto device = physical_device.init_device();
	auto allocator = device.init_allocator();

	//* ---------------> COMMAND POOL INIT

	auto descriptor_set_layout = device.init_descriptor_set_layout({ 
		lvk::descriptor_set_layout_binding(0, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
	});

	auto descriptor_pool = device.init_descriptor_pool(10, {{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }});

	Frame frame_array[FRAMES_IN_FLIGHT];
	

	for (auto& frame: frame_array) {
		frame.command_pool = device.init_command_pool();
		frame.command_buffer = frame.command_pool.init_command_buffer();

		frame.render_fence = device.init_fence();

		frame.render_semaphore = device.init_semaphore();
		frame.present_semaphore = device.init_semaphore();

		frame.camera_buffer = allocator.init_uniform_buffer<mvp_matrix>();
		frame.global_descriptor = descriptor_pool.init_descriptor_set(&descriptor_set_layout);

		frame.global_descriptor.update(&frame.camera_buffer);
	}
	
	//* ---------------> SWAPCHAIN INIT

	auto swapchain = device.init_swapchain(window.size.x, window.size.y, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	});
	auto* depth_image_views = new lvk_image_view[swapchain._image_count];
	auto* depth_images = new lvk_image[swapchain._image_count];
	auto* framebuffers = new lvk_framebuffer[swapchain._image_count];

	auto render_pass = device.init_default_render_pass(swapchain._surface_format.format);

	for (int i = 0; i < swapchain._image_count; i++) {
		depth_images[i] = allocator.init_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TYPE_2D, { swapchain._extent.width, swapchain._extent.height, 1 });
		depth_image_views[i] = depth_images[i].init_image_view(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D);

		framebuffers[i] = render_pass.init_framebuffer(swapchain._extent, { swapchain._image_views[i], depth_image_views[i]._image_view });
	}

	//* ---------------> PIPELINE INIT

	auto pipeline_layout = device.init_pipeline_layout(
		{
			{
				VK_SHADER_STAGE_VERTEX_BIT,
				0,
				sizeof(lucy::MeshPushConstants),
			}
		},
		{
			descriptor_set_layout._descriptor_set_layout
		}
	);

	auto vertex_shader = device.init_shader_module(VK_SHADER_STAGE_VERTEX_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/mesh.vert.spv");
	auto fragment_shader = device.init_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/colored_triangle.frag.spv");
	auto vertex_layout = lucy::Vertex::get_vertex_description();
	
	lvk_pipeline graphics_pipeline = {};
	{
		lvk::config::graphics_pipeline config = {
			.shader_stage_array = {
				lvk::info::shader_stage(&vertex_shader, nullptr),
				lvk::info::shader_stage(&fragment_shader, nullptr),
			},

			.vertex_input_state = lvk::info::vertex_input_state(
				{
					{
						.binding = 0,
						.stride = sizeof(lucy::Vertex),
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
			),
			.input_assembly_state = lvk::info::input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false),
			.rasterization_state = lvk::info::rasterization_state(VK_POLYGON_MODE_FILL),
			.multisample_state = lvk::info::multisample_state(),
			.depth_stencil_state = lvk::info::depth_stencil_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL),
			.color_blend_attachment = lvk::color_blend_attachment(),
			
			.viewport = {
				.x = 0.0f,
				.y = 0.0f,
				.width = (float)window.size.x,
				.height = (float)window.size.y,
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			},

			.scissor = {
				.offset = { 0, 0 },
				.extent = { static_cast<uint32_t>(window.size.x), static_cast<uint32_t>(window.size.y) }
			}
		};

		graphics_pipeline = pipeline_layout.init_graphics_pipeline(&render_pass, &config);
	}

	lucy::Mesh monkey_mesh;
	monkey_mesh.load_obj("/home/laperex/Programming/C++/LucyVK/src/assets/monkey.obj");
	monkey_mesh.vertex_buffer = allocator.init_vertex_buffer(monkey_mesh._vertices.data(), monkey_mesh._vertices.size() * sizeof(monkey_mesh._vertices[0]));

	lucy::Camera camera;
	camera.width = window.size.x;
	camera.height = window.size.y;

	VkClearValue clear_value[2] = {
		{
			.color = { { 0.0f, 0.0f, 0, 0.0f } }
		},
		{
			.depthStencil = {
				.depth = 1.0f
			}
		},
	};

	uint32_t framenumber = 0;
	double dt = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		camera.Update(dt);

		// TODO: Shader Module creation/deletion/query management
		// TODO: vk_shaders.h
		// TODO: maybe restructure init_ based creation
		// TODO: Remove deletion_queue and find a better approach
		// TODO: Better approach for PhysicalDevice selection and Initialization
		// TODO: Add delta time Macros
		// *TODO: multi_init feature for initialization of multiple vulkan types
		
		// TODO! IMPLEMENT COMPUTE SHADERS 

		{
			{
				auto& current_frame = frame_array[framenumber % FRAMES_IN_FLIGHT];
				auto& cmd = current_frame.command_buffer;

				uint32_t image_index;
				swapchain.acquire_next_image(&image_index, current_frame.present_semaphore._semaphore, VK_NULL_HANDLE);
				current_frame.image_index = image_index;
				
				cmd.reset();

				cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

				cmd.begin_render_pass(&framebuffers[image_index], VK_SUBPASS_CONTENTS_INLINE, {
					{
						.color = { { 0.0f, 0.0f, 0, 0.0f } }
					},
					{
						.depthStencil = {
							.depth = 1.0f
						}
					},
				});

				vkCmdBindPipeline(cmd._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline._pipeline);
				
				cmd.bind_vertex_buffers({ monkey_mesh.vertex_buffer._buffer }, { 0 });

				glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(framenumber * 0.4f), glm::vec3(0, 1, 0));

				glm::mat4 mesh_matrix = camera.projection * camera.view * model;

				lucy::MeshPushConstants constants;
				constants.render_matrix = mesh_matrix;
				constants.offset = { 0, 0, 0, 0};

				mvp_matrix mvp = {
					.projection = camera.projection,
					.view = camera.view,
					.model = model
				};

				current_frame.camera_buffer.upload(mvp);

				// vkCmdPushConstants(cmd._command_buffer, pipeline_layout._pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(lucy::MeshPushConstants), &constants);
				vkCmdBindDescriptorSets(cmd._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout._pipeline_layout, 0, 1, &current_frame.global_descriptor._descriptor_set, 0, nullptr);

				vkCmdDraw(cmd._command_buffer, monkey_mesh._vertices.size(), 1, 0, 0);

				cmd.end_render_pass();
				cmd.end();
			}

			if (framenumber > 0) {
				auto& prev_frame = frame_array[(framenumber - 1) % FRAMES_IN_FLIGHT];
				
				VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

				VkSubmitInfo submit = {
					.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					
					.waitSemaphoreCount = 1,
					.pWaitSemaphores = &prev_frame.present_semaphore._semaphore,

					.pWaitDstStageMask = &waitStage,

					.commandBufferCount = 1,
					.pCommandBuffers = &prev_frame.command_buffer._command_buffer,

					.signalSemaphoreCount = 1,
					.pSignalSemaphores = &prev_frame.render_semaphore._semaphore,
				};

				device.submit(&submit, 1, &prev_frame.render_fence);
				
				VkPresentInfoKHR presentInfo = {
					.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
					
					.waitSemaphoreCount = 1,
					.pWaitSemaphores = &prev_frame.render_semaphore._semaphore,
					
					.swapchainCount = 1,
					.pSwapchains = &swapchain._swapchain,
					
					.pImageIndices = &prev_frame.image_index,
				};

				device.present(&presentInfo);
			}
		}

		framenumber += 1;
		// window.SwapWindow();
		// if (framenumber == 20) lucy::Events::IsQuittable() = true;

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
	
	device.wait_idle();

	window.Destroy();
}