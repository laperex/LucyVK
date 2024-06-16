#pragma once

#include "lucyvk/types.h"
#include <vulkan/vulkan_core.h>


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


struct lvk_swapchain {
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	VkExtent2D _extent = { 0, 0 };
	VkSurfaceFormatKHR _surface_format;
	VkPresentModeKHR _present_mode;
	VkImageUsageFlags _image_usage;

	uint32_t _image_count;
	VkImage* _images;
	VkImageView* _image_views;
};