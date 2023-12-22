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
	auto render_pass = device.init_render_pass();

	auto* swapchain = device.create_swapchain(window.size.x, window.size.y);
	auto* framebuffer = render_pass.create_framebuffer(window.size.x, window.size.y, swapchain->_image_view_array);
	
	auto render_fence = device.init_fence(VK_FENCE_CREATE_SIGNALED_BIT);
	auto present_semaphore = device.init_semaphore();
	auto render_semaphore = device.init_semaphore();

	double dt = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		window.SwapWindow();

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
	
	render_pass.destroy_framebuffer(framebuffer);
	device.destroy_swapchain(swapchain);

	device.wait_idle();

	window.Destroy();
}