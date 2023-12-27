#pragma once

#include "lucyvk/vk_types.h"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lvk {
	lvk::swapchain_support_details query_swapchain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
	lvk::queue_family_indices query_queue_family_indices(VkPhysicalDevice physicalDevice, VkSurfaceKHR _surfaceKHR);

	VkPhysicalDevice default_physical_device(const std::vector<VkPhysicalDevice>& physicalDeviceArray, const lvk_instance* instance);
	
	VkImageViewCreateInfo image_view_create_info(VkImage image, VkFormat format, VkImageViewType view_type, VkImageSubresourceRange subresource_range, VkComponentMapping components);
	VkImageViewCreateInfo image_view_create_info(VkImage image, VkFormat format, VkImageAspectFlags aspect_flag, VkImageViewType view_type);
	
	const char* to_string(const VkBufferUsageFlagBits usage);
}
