#include "lucyvk/vk_config.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Camera.h"
#include "Mesh.h"
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

#include <vulkan/vulkan_core.h>

#include <util/logger.h>
#include <lucyvk/vk_static.h>

typedef uint32_t lve_vertex;

#define LVE_CHUNK_SIZE (32)

#define LVE_VERTEX(x, y, z, n, u, v) ((x * LVE_CHUNK_SIZE * LVE_CHUNK_SIZE) + y * LVE_CHUNK_SIZE + z) & 0x7fff

static lvk::vertex_input_description lve_vertex_description() {
	lvk::vertex_input_description description;

	//we will have just 1 vertex buffer binding, with a per-vertex rate
	VkVertexInputBindingDescription mainBinding = {};
	mainBinding.binding = 0;
	mainBinding.stride = sizeof(lve_vertex);
	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	description.bindings.push_back(mainBinding);

	//Position will be stored at Location 0
	VkVertexInputAttributeDescription positionAttribute = {};
	positionAttribute.binding = 0;
	positionAttribute.location = 0;
	positionAttribute.format = VK_FORMAT_R32_UINT;
	positionAttribute.offset = 0;

	description.attributes.push_back(positionAttribute);

	return description;
}

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

struct Frame {
	lvk_fence render_fence;
	lvk_semaphore present_semaphore;
	lvk_semaphore render_semaphore;

	lvk_command_pool command_pool;
	lvk_command_buffer command_buffer;
	
	uint32_t image_index;
};

static constexpr const int FRAMES_IN_FLIGHT = 2;

// TODO: Add delta time Macros

int main(int count, char** args) {
	lucy::Window window = {};
	window.InitWindow();

	auto instance = lvk::initialize("Lucy", window.sdl_window, true);
	auto physical_device = instance.init_physical_device();
	auto device = physical_device.init_device();
	auto allocator = device.init_allocator();
	auto render_pass = device.init_render_pass();

	auto swapchain = device.init_swapchain(window.size.x, window.size.y);
	
	Frame frame[FRAMES_IN_FLIGHT];
	
	for (int i = 0; i < std::size(frame); i++) {
		frame[i].command_pool = device.init_command_pool();
		frame[i].command_buffer = frame[i].command_pool.init_command_buffer();

		frame[i].render_fence = device.init_fence();

		frame[i].render_semaphore = device.init_semaphore();
		frame[i].present_semaphore = device.init_semaphore();
	}

	VkClearValue clearValue[2] = {
		{
			.color = { { 0.0f, 0.0f, 0, 0.0f } }
		},
		{
			.depthStencil = {
				.depth = 1.0f
			}
		}
	};

	VkPushConstantRange push_constant = {
		.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		.offset = 0,
		.size = sizeof(lucy::MeshPushConstants),
	};

	lvk_pipeline_layout pipeline_layout = device.init_pipeline_layout(&push_constant, 1);

	auto vertex_shader = device.init_shader_module(VK_SHADER_STAGE_VERTEX_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/mesh.vert.spv");
	auto fragment_shader = device.init_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/colored_triangle.frag.spv");


	lvk_pipeline graphics_pipeline = {};
	{
		auto vertex_layout = lucy::Vertex::get_vertex_description();

		lvk::config::graphics_pipeline config = {
			.shader_stage_array = {
				lvk::info::shader_stage(&vertex_shader, nullptr),
				lvk::info::shader_stage(&fragment_shader, nullptr),
			},

			.vertex_input_state = lvk::info::vertex_input_state(&vertex_layout),
			.input_assembly_state = lvk::info::input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false),
			.rasterization_state = lvk::info::rasterization_state(VK_POLYGON_MODE_FILL),
			.multisample_state = lvk::info::multisample_state(),
			.depth_stencil_state = lvk::info::depth_stencil_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL),
			.color_blend_attachment = lvk::color_blend_attachment(),
			
			.viewport = {
				.x = 0.0f,
				.y = 0.0f,
				.width = (float)swapchain._extent.width,
				.height = (float)swapchain._extent.height,
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			},

			.scissor = {
				.offset = { 0, 0 },
				.extent = swapchain._extent
			}
		};

		graphics_pipeline = pipeline_layout.init_graphics_pipeline(&render_pass, &config);
	}

	auto* framebuffers = new lvk_framebuffer[swapchain._image_count];
	auto* depth_images = new lvk_image[swapchain._image_count];
	auto* depth_image_views = new lvk_image_view[swapchain._image_count];

	for (int i = 0; i < swapchain._image_count; i++) {
		depth_images[i] = allocator.init_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TYPE_2D, { swapchain._extent.width, swapchain._extent.height, 1 });
		depth_image_views[i] = depth_images[i].init_image_view(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D);

		VkImageView image_view[2] = { swapchain._image_views[i], depth_image_views[i]._image_view };
		framebuffers[i] = render_pass.init_framebuffer(swapchain._extent, image_view, 2);
	}

	lucy::Mesh monkey_mesh;
	monkey_mesh.load_obj("/home/laperex/Programming/C++/LucyVK/src/assets/monkey.obj");
	monkey_mesh.vertex_buffer = allocator.init_vertex_buffer(monkey_mesh._vertices.data(), monkey_mesh._vertices.size() * sizeof(monkey_mesh._vertices[0]));

	lucy::Camera camera;
	camera.width = swapchain._extent.width;
	camera.height = swapchain._extent.height;

	uint32_t framenumber = 0;

	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	double dt = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		camera.Update(dt);

		{
			// auto t0 = std::chrono::high_resolution_clock::now();
			{
				auto& current_frame = frame[framenumber % FRAMES_IN_FLIGHT];
				uint32_t image_index;
				swapchain.acquire_next_image(&image_index, current_frame.present_semaphore._semaphore, VK_NULL_HANDLE);
				current_frame.image_index = image_index;

				current_frame.command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
				current_frame.command_buffer.begin_render_pass(&framebuffers[image_index], clearValue, 2, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(current_frame.command_buffer._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline._pipeline);
				VkDeviceSize offset = 0;
				vkCmdBindVertexBuffers(current_frame.command_buffer._command_buffer, 0, 1, &monkey_mesh.vertex_buffer._buffer, &offset);

				glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(framenumber * 0.4f), glm::vec3(0, 1, 0));

				glm::mat4 mesh_matrix = camera.projection * camera.view * model;

				lucy::MeshPushConstants constants;
				constants.render_matrix = mesh_matrix;
				constants.offset = { 0, 0, 0, 0};

				vkCmdPushConstants(current_frame.command_buffer._command_buffer, pipeline_layout._pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(lucy::MeshPushConstants), &constants);

				vkCmdDraw(current_frame.command_buffer._command_buffer, monkey_mesh._vertices.size(), 1, 0, 0);

				current_frame.command_buffer.end_render_pass();
				current_frame.command_buffer.end();
			}
			// auto t1 = std::chrono::high_resolution_clock::now();

			if (framenumber > 0) {
				auto& prev_frame = frame[(framenumber - 1) % FRAMES_IN_FLIGHT];
				
				VkSubmitInfo submit = {};
				submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit.pNext = nullptr;

				VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

				submit.pWaitDstStageMask = &waitStage;

				submit.waitSemaphoreCount = 1;
				submit.pWaitSemaphores = &prev_frame.present_semaphore._semaphore;

				submit.signalSemaphoreCount = 1;
				submit.pSignalSemaphores = &prev_frame.render_semaphore._semaphore;

				submit.commandBufferCount = 1;
				submit.pCommandBuffers = &prev_frame.command_buffer._command_buffer;

				//submit command buffer to the queue and execute it.
				// _renderFence will now block until the graphic commands finish execution
				vkQueueSubmit(device._graphicsQueue, 1, &submit, prev_frame.render_fence._fence);

				// auto x0 = std::chrono::high_resolution_clock::now();

				prev_frame.render_fence.wait();
				prev_frame.render_fence.reset();

				prev_frame.command_buffer.reset();
				
				
				VkPresentInfoKHR presentInfo = {};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				presentInfo.pNext = nullptr;

				presentInfo.pSwapchains = &swapchain._swapchain;
				presentInfo.swapchainCount = 1;

				presentInfo.pWaitSemaphores = &prev_frame.render_semaphore._semaphore;
				presentInfo.waitSemaphoreCount = 1;

				presentInfo.pImageIndices = &prev_frame.image_index;

				// auto x1 = std::chrono::high_resolution_clock::now();

				// dloggln("Wait: ", std::chrono::duration_cast<std::chrono::microseconds>(x1 - x0).count());

				vkQueuePresentKHR(device._graphicsQueue, &presentInfo);
			}

			// auto t2 = std::chrono::high_resolution_clock::now();

			// dloggln("Record: ", std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count(), " | Submit: ", std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
		}

		framenumber += 1;
		// window.SwapWindow();
		// if (framenumber == 20) lucy::Events::IsQuittable() = true;

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
	
	// vmaDestroyBuffer(allocator->_allocator, _buffer, _allocation);

	device.wait_idle();

	window.Destroy();
}