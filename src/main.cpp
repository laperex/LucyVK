// #include "lucyvk/Instance.h"
// #include "lucyvk/PhysicalDevice.h"
// #include "lucyvk/LogicalDevice.h"
// #include "lucyvk/CommandPool.h"
// #include "lucyvk/ImageView.h"
// #include "lucyvk/Swapchain.h"
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

#include <vulkan/vulkan_core.h>

#include <util/logger.h>
#include <lucyvk/vk_static.h>


struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
};

struct Mesh {
	std::vector<Vertex> _vertices;

	// AllocatedBuffer _vertexBuffer;
};


int main(int count, char** args) {
	SDL_Init(SDL_INIT_VIDEO);

	lucy::Window window = {};
	window.InitWindow();

	auto instance = lvk::initialize("Lucy", window.sdl_window, true);
	auto physical_device = instance.init_physical_device();
	auto device = physical_device.init_device();
	auto command_pool = device.init_command_pool();
	auto render_pass = device.init_render_pass();

	auto* swapchain = device.create_swapchain(window.size.x, window.size.y);
	auto* framebuffer = render_pass.create_framebuffer(window.size.x, window.size.y, swapchain->_image_view_array);
	
	auto render_fence = device.init_fence(1, VK_FENCE_CREATE_SIGNALED_BIT);
	auto present_semaphore = device.init_semaphore(1);
	auto render_semaphore = device.init_semaphore(1);
	
	auto command_buffers = command_pool.init_command_buffer(1);
	// auto command_buffers = command_pool.init_command_buffer(swapchain->_images.size());

	command_buffers.reset();
	
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkClearValue clearValue;
	// float flash = abs(sin(_frameNumber / 120.f));
	clearValue.color = { { 0.0f, 0.0f, 1, 1.0f } };

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = {};
	rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpInfo.pNext = nullptr;

	rpInfo.renderPass = render_pass._render_pass;
	rpInfo.renderArea.offset.x = 0;
	rpInfo.renderArea.offset.y = 0;
	rpInfo.renderArea.extent = swapchain->_extent;

	//connect clear values
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	auto vertex_shader = device.init_shader_module(VK_SHADER_STAGE_VERTEX_BIT, "shaders/colored_triangle.vert.spv");
	auto fragment_shader = device.init_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/colored_triangle.frag.spv");

	lvk::graphics_pipeline_config config;
	{
		config.shader_stage_array.push_back(lvk::shader_stage_create_info(&vertex_shader, nullptr));
		config.shader_stage_array.push_back(lvk::shader_stage_create_info(&fragment_shader, nullptr));
		
		config.color_blend_attachment = lvk::color_blend_attachment();
		// config.color_blend_state = lvk::color_blend_state(nullptr)
		config.vertex_input_state = lvk::vertex_input_state_create_info(nullptr, 0, nullptr, 0);
		config.input_assembly_state = lvk::input_assembly_state_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false);
		config.rasterization_state = lvk::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
		config.multisample_state = lvk::multisample_state_create_info();

		config.viewport.x = 0.0f;
		config.viewport.y = 0.0f;
		config.viewport.width = (float)swapchain->_extent.width;
		config.viewport.height = (float)swapchain->_extent.height;
		config.viewport.minDepth = 0.0f;
		config.viewport.maxDepth = 1.0f;

		config.scissor.offset = { 0, 0 };
		config.scissor.extent = swapchain->_extent;
	}

	auto pipeline_layout = device.init_pipeline_layout();

	auto graphics_pipeline = pipeline_layout.init_graphics_pipeline(&render_pass, &config);

	double dt = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		{
			render_fence.wait();
			render_fence.reset();
			uint32_t image_index = swapchain->acquire_next_image(present_semaphore._semaphore[0], nullptr);

			{
				rpInfo.framebuffer = framebuffer->_framebuffer_array[image_index];

				command_buffers.cmd_begin(0, &cmdBeginInfo);
				command_buffers.cmd_render_pass_begin(0, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
				
				vkCmdBindPipeline(command_buffers._command_buffers[0], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline._pipeline);
				vkCmdDraw(command_buffers._command_buffers[0], 3, 1, 0, 0);

				command_buffers.cmd_render_pass_end(0);				
				command_buffers.cmd_end(0);
			}

			{
				VkSubmitInfo submit = {};
				submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit.pNext = nullptr;

				VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

				submit.pWaitDstStageMask = &waitStage;

				submit.waitSemaphoreCount = 1;
				submit.pWaitSemaphores = present_semaphore._semaphore;

				submit.signalSemaphoreCount = 1;
				submit.pSignalSemaphores = render_semaphore._semaphore;

				submit.commandBufferCount = command_buffers._count;
				submit.pCommandBuffers = command_buffers._command_buffers;

				//submit command buffer to the queue and execute it.
				// _renderFence will now block until the graphic commands finish execution
				assert(vkQueueSubmit(device._graphicsQueue, 1, &submit, render_fence._fence[0]) == VK_SUCCESS);
				
				VkPresentInfoKHR presentInfo = {};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				presentInfo.pNext = nullptr;

				presentInfo.pSwapchains = &swapchain->_swapchain;
				presentInfo.swapchainCount = 1;

				presentInfo.pWaitSemaphores = render_semaphore._semaphore;
				presentInfo.waitSemaphoreCount = 1;

				presentInfo.pImageIndices = &image_index;

				assert(vkQueuePresentKHR(device._graphicsQueue, &presentInfo) == VK_SUCCESS);
			}
		}

		window.SwapWindow();

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
	
	render_fence.wait(1000000000);

	device.wait_idle();

	window.Destroy();
}