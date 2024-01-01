#include "lucyvk/vk_physical_device.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_instance.h"
#include "util/logger.h"
#include <stdexcept>


// |--------------------------------------------------
// ----------------> PHYSICAL DEVICE
// |--------------------------------------------------

lvk_physical_device lvk_instance::init_physical_device(lvk::SelectPhysicalDeviceFunction function) {
	lvk_physical_device physical_device = {
		.instance = this
	};
	
	uint32_t availableDeviceCount = 0;

	vkEnumeratePhysicalDevices(_instance, &availableDeviceCount, VK_NULL_HANDLE);
	std::vector<VkPhysicalDevice> availableDevices(availableDeviceCount);
	vkEnumeratePhysicalDevices(_instance, &availableDeviceCount, availableDevices.data());

	physical_device._physical_device = (function == nullptr) ?
		lvk::default_physical_device(availableDevices, this):
		function(availableDevices, this);
	
	if (physical_device._physical_device == nullptr) {
		throw std::runtime_error("failed to find suitable PhysicalDevice!");
	}

	physical_device._queue_family_indices = lvk::query_queue_family_indices(physical_device._physical_device, _surface);
	physical_device._swapchain_support_details = lvk::query_swapchain_support_details(physical_device._physical_device, _surface);

	vkGetPhysicalDeviceFeatures(physical_device._physical_device, &physical_device._features);
	vkGetPhysicalDeviceProperties(physical_device._physical_device, &physical_device._properties);
	
	dloggln(physical_device._physical_device, " Physical Device - ", physical_device._properties.deviceName);

	return physical_device;
}