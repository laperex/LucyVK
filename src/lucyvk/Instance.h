#pragma once

#include <SDL_vulkan.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <lucyvk/types.h>

namespace lucyvk {
	struct Instance {
		VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR _surface = VK_NULL_HANDLE;
		VkInstance _instance = VK_NULL_HANDLE;

		Instance();
		Instance(const char* name, SDL_Window* sdl_window, bool debug_mode, std::vector<const char*> layers = {});
		~Instance();

		PhysicalDevice CreatePhysicalDevice();
		Device CreateLogicalDevice(const PhysicalDevice& physicalDevice);
	};
}