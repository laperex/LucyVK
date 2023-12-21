#pragma once

#include <vulkan/vulkan_core.h>

namespace lvk {
	struct vk_init {
		
	};

	bool init_instance(VkDebugUtilsMessengerEXT _debugMessenger, VkSurfaceKHR _surface, VkInstance _instance, std::vector<const char*> layers = {});
}