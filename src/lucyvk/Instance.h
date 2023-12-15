#pragma once

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "device.h"

namespace lucyvk {
	struct Instance {
		Instance();
		~Instance();
		
		VkInstance _instance;
		VkDebugUtilsMessengerEXT _debugMessenger;
		VkSurfaceKHR _surface;
		
		bool Initialize(const char* name, SDL_Window* sdl_window);

		LogicalDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice);

	private:
		const char* layers[1] = { "VK_LAYER_KHRONOS_validation" };
		const char* deviceExtensions[1] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		// std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		// std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}