#include <set>
#include <vulkan/vulkan_core.h>
#include <string>
#include <lucyvk/PhysicalDevice.h>
#include <lucyvk/Instance.h>
#include <lucyvk/LogicalDevice.h>
#include <util/logger.h>

namespace lucyvk {
	SwapchainSupportDetails QuerySwapchainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR _surfaceKHR) {
		SwapchainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surfaceKHR, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surfaceKHR, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surfaceKHR, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surfaceKHR, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surfaceKHR, &presentModeCount, details.presentModes.data());
		}
		return details;
	}

	static lucyvk::QueueFamilyIndices QueryQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR _surfaceKHR) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		QueueFamilyIndices indices;

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

	static VkPhysicalDevice DefaultPhysicalDeviceSelection(const std::vector<VkPhysicalDevice>& physicalDeviceArray, const lucyvk::Instance& instance) {
		for (const auto& physicalDevice: physicalDeviceArray) {
			bool isRequiredDeviceExtensionsAvailable = false;
			bool isIndicesComplete = false;
			bool isSwapchainAdequate = false;

			{
				uint32_t availableExtensionCount;
				vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);
				std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
				vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensions.data());

				for (const auto& extension: availableExtensions) {
					if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
						isRequiredDeviceExtensionsAvailable = true;
						break;
					}
				}
			}

			{
				isIndicesComplete = QueryQueueFamilyIndices(physicalDevice, instance._surface);

				if (isRequiredDeviceExtensionsAvailable) {
					SwapchainSupportDetails swapchainSupport = QuerySwapchainSupportDetails(physicalDevice, instance._surface);
					isSwapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
				}

				VkPhysicalDeviceFeatures supportedFeatures;
				vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

				if (isIndicesComplete && isRequiredDeviceExtensionsAvailable && isSwapchainAdequate && supportedFeatures.samplerAnisotropy) {
					return physicalDevice;
				}
			}
		}

		return nullptr;
	}
}

lucyvk::PhysicalDevice::PhysicalDevice(const Instance& instance)
	: instance(instance)
{
	Initialize();
}

lucyvk::PhysicalDevice::~PhysicalDevice()
{
	Destroy();
}

bool lucyvk::PhysicalDevice::Initialize(SelectPhysicalDeviceFunction selectPhysicalDeviceFunction) {
	uint32_t availableDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance._instance, &availableDeviceCount, nullptr);
	std::vector<VkPhysicalDevice> availableDevices(availableDeviceCount);
	vkEnumeratePhysicalDevices(instance._instance, &availableDeviceCount, availableDevices.data());

	_physicalDevice = (selectPhysicalDeviceFunction == nullptr) ?
		DefaultPhysicalDeviceSelection(availableDevices, instance):
		selectPhysicalDeviceFunction(availableDevices, instance);
	
	if (_physicalDevice == nullptr) {
		throw std::runtime_error("failed to find suitable PhysicalDevice!");
	}

	_queueFamilyIndices = QueryQueueFamilyIndices(_physicalDevice, instance._surface);
	_swapchainSupportDetails = QuerySwapchainSupportDetails(_physicalDevice, instance._surface);

	vkGetPhysicalDeviceFeatures(_physicalDevice, &_features);
	vkGetPhysicalDeviceProperties(_physicalDevice, &_properties);
	
	dloggln("Physical Device - ", _properties.deviceName);

	return true;
}

bool lucyvk::PhysicalDevice::Destroy() {

	return true;
}

lucyvk::Device lucyvk::PhysicalDevice::CreateLogicalDevice() {
	return { instance, *this };
}

const VkFormat lucyvk::PhysicalDevice::FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const {
	for (VkFormat format: candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
		if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

const uint32_t lucyvk::PhysicalDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags) const {
	VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags) {
            return i;
        }
    }

    throw std::runtime_error("FAILED TO FIND SUITABLE MEMORY TYPE!");
}

