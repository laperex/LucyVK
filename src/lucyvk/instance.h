#pragma once

#include "lucyvk/config.h"
// #include "lucyvk/physical_device.h"
// #include "dev"


// |--------------------------------------------------
// ----------------> INSTANCE
// |--------------------------------------------------


struct lvk_instance {
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkSurfaceKHR _surface;
	VkInstance _instance;

	std::vector<const char*> layers = {};

	~lvk_instance();

	static lvk_instance init(const lvk::config::instance* config, SDL_Window* sdl_window);
	
	bool is_debug_enable();

	lvk_device_ptr init_device(std::vector<const char*> extensions, std::vector<const char*> layers = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }, lvk::SelectPhysicalDeviceFunction function = nullptr);
	// lvk_physical_device init_physical_device();

	void destroy();
};