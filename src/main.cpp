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
	auto main_command_buffer = command_pool.init_command_buffer();

	auto* swapchain = device.create_swapchain(window.size.x, window.size.y);

	// VkCommandBufferBeginInfo beginInfo = {};
	
	// beginInfo.pInheritanceInfo = nullptr;
	// beginInfo.flags
	
	// vkBeginCommandBuffer(main_command_buffer._command_buffer, beginInfo);
	
	// VkSubpassDescription desc;
	
	// VkRenderPassCreateInfo createInfo = {};
	// createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	// createInfo.pNext = nullptr;
	// createInfo.flags = 0;
	// createInfo.attachmentCount = 
	// createInfo.pAttachments = 
	// createInfo.subpassCount = 
	// createInfo.pSubpasses = 
	// createInfo.dependencyCount = 
	// createInfo.pDependencies = 
	// info.pSubpasses
	
	// vkCreateRenderPass(device, )

	double dt = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		window.SwapWindow();

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
	
	device.destroy_swapchain(swapchain);

	device.wait_idle();

	window.Destroy();
}