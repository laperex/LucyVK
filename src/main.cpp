// #include "lucyvk/Instance.h"
// #include "lucyvk/PhysicalDevice.h"
// #include "lucyvk/LogicalDevice.h"
// #include "lucyvk/CommandPool.h"
// #include "lucyvk/ImageView.h"
// #include "lucyvk/Swapchain.h"
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
	
	auto render_fence = device.init_fence(VK_FENCE_CREATE_SIGNALED_BIT);
	auto present_semaphore = device.init_semaphore();
	auto render_semaphore = device.init_semaphore();

	auto command_buffers = command_pool.init_command_buffer(1);

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
	
	
	uint32_t frame = 0;

	double dt = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		{
			render_fence.wait();
			render_fence.reset();
			uint32_t image_index = swapchain->acquire_next_image(1000000000, present_semaphore._semaphore);
			
			{
				assert(vkBeginCommandBuffer(command_buffers._command_buffer, &cmdBeginInfo) == VK_SUCCESS);

				rpInfo.framebuffer = framebuffer->_framebuffer_array[image_index];
				
				vkCmdBeginRenderPass(command_buffers._command_buffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
				
				vkCmdEndRenderPass(command_buffers._command_buffer);
				
				assert(vkEndCommandBuffer(command_buffers._command_buffer) == VK_SUCCESS);
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
				submit.pCommandBuffers = &command_buffers._command_buffer;

				//submit command buffer to the queue and execute it.
				// _renderFence will now block until the graphic commands finish execution
				assert(vkQueueSubmit(device._graphicsQueue, 1, &submit, render_fence._fence) == VK_SUCCESS);
				
				VkPresentInfoKHR presentInfo = {};
				presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
				presentInfo.pNext = nullptr;

				presentInfo.pSwapchains = &swapchain->_swapchain;
				presentInfo.swapchainCount = 1;

				presentInfo.pWaitSemaphores = &render_semaphore._semaphore;
				presentInfo.waitSemaphoreCount = 1;

				presentInfo.pImageIndices = &image_index;

				assert(vkQueuePresentKHR(device._graphicsQueue, &presentInfo) == VK_SUCCESS);
			}
		}
		
		frame++;
		clearValue.color.float32[0] = float(frame % 4) / 2.0f;

		window.SwapWindow();

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
	
	render_fence.wait(1000000000);
	
	render_pass.destroy_framebuffer(framebuffer);
	device.destroy_swapchain(swapchain);

	device.wait_idle();

	window.Destroy();
}