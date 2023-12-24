#pragma once

#include "lucyvk/vk_static.h"
#include <vector>
#include <vulkan/vulkan_core.h>
namespace lvk {
	lvk::swapchain_support_details query_swapchain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
	lvk::queue_family_indices query_queue_family_indices(VkPhysicalDevice physicalDevice, VkSurfaceKHR _surfaceKHR);

	VkPhysicalDevice default_physical_device(const std::vector<VkPhysicalDevice>& physicalDeviceArray, const lvk_instance* instance);
}
