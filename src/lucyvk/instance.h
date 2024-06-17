#pragma once

#include "lucyvk/config.h"
#include "lucyvk/handles.h"
#include "lucyvk/functions.h"


// |--------------------------------------------------
// ----------------> INSTANCE
// |--------------------------------------------------


struct lvk_instance {
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkSurfaceKHR _surfaceKHR;
	VkInstance _instance;

	std::vector<const char*> layers = {};

	~lvk_instance();

	static lvk_instance init(const lvk::config::instance* config, SDL_Window* sdl_window);
	
	bool is_debug_enable();

	lvk_device create_device(std::vector<const char*> extensions, std::vector<const char*> layers = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }, lvk::SelectPhysicalDevice_F function = nullptr);

	void destroy();
};