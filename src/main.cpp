#include "lucyvk/Instance.h"
#include "lucyvk/PhysicalDevice.h"
#include "lucyvk/LogicalDevice.h"
#include "lucyvk/CommandPool.h"
#include "lucyvk/Swapchain.h"
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


int main(int count, char** args) {
	SDL_Init(SDL_INIT_VIDEO);

	lucy::Window window = {};
	window.InitWindow();
	
	const std::vector<const char*> layers = {};
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	lucyvk::Instance instance = lucyvk::Initialize("Lucy Framework", window.sdl_window, true, layers);

	lucyvk::PhysicalDevice physicalDevice = { instance };
	
	lucyvk::Device device = { physicalDevice, deviceExtensions, layers };

	auto swapchain = std::make_unique<lucyvk::Swapchain>(device, VkExtent2D { static_cast<uint32_t>(window.size.x), static_cast<uint32_t>(window.size.y) } );

	lucyvk::CommandPool commandPool = {
		device,
		VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
	};

	double dt = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		window.SwapWindow();

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}

	device.WaitIdle();

	window.Destroy();
}