#include "PhysicalDevice.h"
#include <vulkan/vulkan_core.h>

static VkPhysicalDevice DefaultPhysicalDeviceSelection(const std::vector<VkPhysicalDevice>& physicalDevice, VkInstance instance, VkSurfaceKHR surface) {
	return nullptr;
}

bool lucyvk::PhysicalDevice::Initialize(SelectPhysicalDeviceFunction selectPhysicalDeviceFunction) {
	uint32_t availableDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance._instance, &availableDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> availableDevices(availableDeviceCount);
	vkEnumeratePhysicalDevices(instance._instance, &availableDeviceCount, availableDevices.data());

	_physicalDevice = (selectPhysicalDeviceFunction == nullptr) ?
		DefaultPhysicalDeviceSelection(availableDevices, instance._instance, instance._surface):
		selectPhysicalDeviceFunction(availableDevices, instance._instance, instance._surface);

	

	return true;
}


