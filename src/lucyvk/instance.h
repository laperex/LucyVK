#pragma once

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include "device.h"

namespace lucyvk {
	struct instance {
		instance();
		~instance();
		
		VkInstance _instance;
		
		bool init(const char* name);

		// physicaldevice physicaldevice();
		logicaldevice createlogicaldevice();

		bool init(const char* name, SDL_Window* sdl_window);

	private:
		const char* layers[1] = { "VK_LAYER_KHRONOS_validation" };
		// std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		// std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};
}