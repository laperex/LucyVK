#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Camera.h"
#include "Mesh.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_pipeline.h"
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


int main(int count, char** args) {
	SDL_Init(SDL_INIT_VIDEO);

	lucy::Window window = {};
	// window.flags |= SDL_WINDOW_RESIZABLE;
	window.InitWindow();

	auto instance = lvk::initialize("Lucy", window.sdl_window, true);
	auto physical_device = instance.init_physical_device();
	auto device = physical_device.init_device();
	auto allocator = device.init_allocator();
	auto command_pool = device.init_command_pool();
	auto render_pass = device.init_render_pass();

	auto swapchain = device.init_swapchain(window.size.x, window.size.y);
	
	auto render_fence = device.init_fence();
	auto present_semaphore = device.init_semaphore();
	auto render_semaphore = device.init_semaphore();
	
	auto command_buffer = command_pool.init_command_buffer();

	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkClearValue clearValue[2];
	clearValue[0].color = { { 0.0f, 0.0f, 0, 0.0f } };
	clearValue[1].depthStencil.depth = 1.0f;

	auto vertex_shader = device.init_shader_module(VK_SHADER_STAGE_VERTEX_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/mesh.vert.spv");
	auto fragment_shader = device.init_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/colored_triangle.frag.spv");

	auto vertex_layout = lucy::Vertex::get_vertex_description();

	lvk::graphics_pipeline_config config;
	{
		config.shader_stage_array.push_back(lvk::shader_stage_create_info(&fragment_shader, nullptr));
		// config.shader_stage_array.push_back(lvk::shader_stage_create_info(&fragment_shader, nullptr));
		config.shader_stage_array.push_back(lvk::shader_stage_create_info(&vertex_shader, nullptr));
		
		config.color_blend_attachment = lvk::color_blend_attachment();
		// config.color_blend_state = lvk::color_blend_state(nullptr)
		config.depth_stencil_state = lvk::depth_stencil_state_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

		config.vertex_input_state = lvk::vertex_input_state_create_info(vertex_layout.bindings.data(), vertex_layout.bindings.size(), vertex_layout.attributes.data(), vertex_layout.attributes.size());
		config.input_assembly_state = lvk::input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);
		config.rasterization_state = lvk::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
		config.multisample_state = lvk::multisample_state_create_info();

		config.viewport.x = 0.0f;
		config.viewport.y = 0.0f;
		config.viewport.width = (float)swapchain._extent.width;
		config.viewport.height = (float)swapchain._extent.height;
		config.viewport.minDepth = 0.0f;
		config.viewport.maxDepth = 1.0f;

		config.scissor.offset = { 0, 0 };
		config.scissor.extent = swapchain._extent;
	}

	VkPushConstantRange push_constant;
	push_constant.offset = 0;
	push_constant.size = sizeof(lucy::MeshPushConstants);
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	auto pipeline_layout = device.init_pipeline_layout(&push_constant, 1);
	auto graphics_pipeline = pipeline_layout.init_graphics_pipeline(&render_pass, &config);

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

	double dt = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		camera.Update(dt);

		{
			uint32_t image_index;
			swapchain.acquire_next_image(&image_index, present_semaphore._semaphore, VK_NULL_HANDLE);

			{
				command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
				command_buffer.begin_render_pass(&framebuffers[image_index], clearValue, 2, VK_SUBPASS_CONTENTS_INLINE);

				VkDeviceSize offset = 0;
				vkCmdBindPipeline(command_buffer._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline._pipeline);
				vkCmdBindVertexBuffers(command_buffer._command_buffer, 0, 1, &monkey_mesh.vertex_buffer._buffer, &offset);

				glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(float(framenumber++) * 0.4f), glm::vec3(0, 1, 0));

				glm::mat4 mesh_matrix = camera.projection * camera.view * model;

				lucy::MeshPushConstants constants;
				constants.render_matrix = mesh_matrix;

				//upload the matrix to the GPU via push constants
				vkCmdPushConstants(command_buffer._command_buffer, pipeline_layout._pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(lucy::MeshPushConstants), &constants);

				//we can now draw
				vkCmdDraw(command_buffer._command_buffer, monkey_mesh._vertices.size(), 1, 0, 0);

				command_buffer.end_render_pass();
				command_buffer.end();
			}

			{
				VkSubmitInfo submit = {};
				submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit.pNext = nullptr;

				VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

				submit.pWaitDstStageMask = &waitStage;

				submit.waitSemaphoreCount = 1;
				submit.pWaitSemaphores = &present_semaphore._semaphore;

				submit.signalSemaphoreCount = 1;
				submit.pSignalSemaphores = &render_semaphore._semaphore;

				submit.commandBufferCount = 1;
				submit.pCommandBuffers = &command_buffer._command_buffer;

				//submit command buffer to the queue and execute it.
				// _renderFence will now block until the graphic commands finish execution
				render_fence.reset();
				vkQueueSubmit(device._graphicsQueue, 1, &submit, render_fence._fence);
				render_fence.wait();
				
				VkPresentInfoKHR presentInfo = {};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				presentInfo.pNext = nullptr;

				presentInfo.pSwapchains = &swapchain._swapchain;
				presentInfo.swapchainCount = 1;

				presentInfo.pWaitSemaphores = &render_semaphore._semaphore;
				presentInfo.waitSemaphoreCount = 1;

				presentInfo.pImageIndices = &image_index;

				vkQueuePresentKHR(device._graphicsQueue, &presentInfo);
			}
		}

		window.SwapWindow();

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
	
	// vmaDestroyBuffer(allocator->_allocator, _buffer, _allocation);

	device.wait_idle();

	window.Destroy();
}