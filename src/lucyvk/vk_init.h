#pragma once

#include <SDL_video.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lvk {
	struct vk_init {
		VkDebugUtilsMessengerEXT _debugMessenger;
		VkSurfaceKHR _surface;
		VkInstance _instance;

		VkPhysicalDevice _physicalDevice;
		
		
		
		struct logical_device {
			VkDevice _device;

			void wait_idle();
		} device;
	};

	vk_init init_instance(SDL_Window* sdl_window, std::vector<const char*> layers = {});
}