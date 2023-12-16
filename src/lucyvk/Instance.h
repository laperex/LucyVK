#pragma once

#include <SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "device.h"

namespace lucyvk {
	struct Instance {
		Instance();
		~Instance();

		VkDebugUtilsMessengerEXT _debugMessenger;
		VkSurfaceKHR _surface;
		VkInstance _instance;

		bool Initialize(const char* name, SDL_Window* sdl_window);

		LogicalDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice);

	private:
		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		const std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
		// std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}