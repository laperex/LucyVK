#pragma once

#include <SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <lucyvk/types.h>

namespace lucyvk {
	struct Instance {
		VkDebugUtilsMessengerEXT _debugMessenger;
		VkSurfaceKHR _surface;
		VkInstance _instance;

		bool Initialize(const char* name, SDL_Window* sdl_window);

		PhysicalDevice CreatePhysicalDevice();
		// LogicalDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice);

		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
	};
}