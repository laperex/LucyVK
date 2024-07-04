#pragma once

#include "lucyvk/config.h"
#include "lucyvk/handles.h"
#include "lucyvk/functions.h"


// |--------------------------------------------------
// ----------------> INSTANCE
// |--------------------------------------------------


struct lvk_instance {
	HANDLE_DEF(VkInstance, _instance)
	VkSurfaceKHR _surfaceKHR;
	VkDebugUtilsMessengerEXT _debug_messenger;

	// std::vector<const char*> layers = {};

	~lvk_instance();

	static lvk_instance initialize(const char* name, SDL_Window* sdl_window, bool enable_validation_layers, std::vector<const char*> layers = {}, std::vector<const char*> extensions = {});
	
	bool is_debug_enable();

	lvk_device create_device(std::vector<const char*> extensions, lvk::SelectPhysicalDevice_F function = nullptr);

	void destroy();
};