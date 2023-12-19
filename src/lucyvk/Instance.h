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

		std::vector<const char*> layers = {};
		
		Instance();
		~Instance();

		bool Initialize(const char* name, SDL_Window* sdl_window);

		PhysicalDevice CreatePhysicalDevice();
		Device CreateLogicalDevice(const PhysicalDevice& physicalDevice);
	};
}