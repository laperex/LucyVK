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

	bool recreate(const uint32_t width, const uint32_t height);
	VkResult acquire_next_image(uint32_t* index, VkSemaphore semaphore, VkFence fence, const uint64_t timeout = LVK_TIMEOUT);
	
	const lvk_device* device;
	const lvk_physical_device* physical_device;
	const lvk_instance* instance;
};