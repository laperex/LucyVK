#pragma once

// #include "lucyvk/types.h"
#include <functional>
#include "lucyvk/handles.h"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lvk {
	typedef std::function<VkPhysicalDevice(const std::vector<VkPhysicalDevice>&, const lvk_instance* instance)> SelectPhysicalDevice_F;

	// VkSurfaceFormatKHR get_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& format_array);
	// lvk::swapchain_support_details query_swapchain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

	VkPhysicalDevice default_physical_device(const std::vector<VkPhysicalDevice>& physicalDeviceArray, const lvk_instance* instance);

	const char* to_string(const VkBufferUsageFlagBits usage);
	const char* to_string(const VkPresentModeKHR mode);
	
	void load_image_data(const char* filename);
}
