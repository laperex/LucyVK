#include "lucyvk/vk_function.h"
#include "util/logger.h"
#include <vulkan/vulkan_core.h>

lvk::swapchain_support_details lvk::query_swapchain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
	lvk::swapchain_support_details details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, VK_NULL_HANDLE);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, VK_NULL_HANDLE);

	if (presentModeCount != 0) {
		details.present_modes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, details.present_modes.data());
	}
	return details;
}

lvk::queue_family_indices lvk::query_queue_family_indices(VkPhysicalDevice physicalDevice, VkSurfaceKHR _surfaceKHR) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, VK_NULL_HANDLE);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	lvk::queue_family_indices indices;

	for (int i = 0; i < queueFamilies.size(); i++) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surfaceKHR, &presentSupport);

		if (queueFamilies[i].queueCount > 0 && presentSupport) {
			indices.present = i;
		}
		
		// if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
		// 	indices.compute = i;
		// }

		// if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
		// 	indices.sparse_binding = i;
		// }

		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics = i;
		}

		if (indices) {
			break;
		}
	}
	
	return indices;
}

VkPhysicalDevice lvk::default_physical_device(const std::vector<VkPhysicalDevice>& physicalDeviceArray, const lvk_instance* instance) {
	dloggln("");
	dloggln("Available GPUs:");
	
	VkPhysicalDeviceProperties properties;
	for (const auto& physical_device: physicalDeviceArray) {
		vkGetPhysicalDeviceProperties(physical_device, &properties);
		
		dloggln(properties.deviceName);
		
		for (const auto& mode: query_swapchain_support_details(physical_device, instance->_surface).present_modes) {
			switch (mode) {
				case VK_PRESENT_MODE_IMMEDIATE_KHR:
					dloggln("	VK_PRESENT_MODE_IMMEDIATE_KHR");
					break;
				case VK_PRESENT_MODE_MAILBOX_KHR:
					dloggln("	VK_PRESENT_MODE_MAILBOX_KHR");
					break;
				case VK_PRESENT_MODE_FIFO_KHR:
					dloggln("	VK_PRESENT_MODE_FIFO_KHR");
					break;
				case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
					dloggln("	VK_PRESENT_MODE_FIFO_RELAXED_KHR");
					break;
				case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
					dloggln("	VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR");
					break;
				case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
					dloggln("	VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR");
					break;
				case VK_PRESENT_MODE_MAX_ENUM_KHR:
					dloggln("	VK_PRESENT_MODE_MAX_ENUM_KHR");
					break;
			}
		}
	}

	dloggln("");

	for (const auto& physicalDevice: physicalDeviceArray) {
		bool isRequiredDeviceExtensionsAvailable = false;
		bool isIndicesComplete = false;
		bool isSwapchainAdequate = false;
		
		return physicalDeviceArray.back();

		{
			uint32_t availableExtensionCount;
			vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &availableExtensionCount, VK_NULL_HANDLE);
			std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
			vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &availableExtensionCount, availableExtensions.data());

			for (const auto& extension: availableExtensions) {
				if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
					isRequiredDeviceExtensionsAvailable = true;
					break;
				}
			}
		}

		{
			isIndicesComplete = query_queue_family_indices(physicalDevice, instance->_surface);

			if (isRequiredDeviceExtensionsAvailable) {
				lvk::swapchain_support_details swapchainSupport = query_swapchain_support_details(physicalDevice, instance->_surface);
				isSwapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.present_modes.empty();
			}

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

			if (isIndicesComplete && isRequiredDeviceExtensionsAvailable && isSwapchainAdequate && supportedFeatures.samplerAnisotropy) {
				return physicalDevice;
			}
		}
	}

	return VK_NULL_HANDLE;
}
