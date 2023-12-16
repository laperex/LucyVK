#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "device.h"
#include "lucyvk/Instance.h"

namespace lucyvk {
	typedef std::function<VkPhysicalDevice(const std::vector<VkPhysicalDevice>&, VkInstance, VkSurfaceKHR)> SelectPhysicalDeviceFunction;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;

		operator bool() const {
			return graphics.has_value() && present.has_value();
		}
	};

	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	
	struct PhysicalDevice {
		VkPhysicalDevice _physicalDevice;

		Instance& instance;
		
		bool Initialize(SelectPhysicalDeviceFunction selectPhysicalDeviceFunction = nullptr);

		std::vector<VkPhysicalDevice> GetAvailablePhysicalDevices();

		bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice);
		QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice physicalDevice);
		
		VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags);

		// SwapchainSupportDetails QuerySwapchainSupportDetails(VkPhysicalDevice device);
		// VkPhysicalDevice PickPhysicalDevice(std::function<bool(VkPhysicalDevice, VkInstance, VkSurfaceKHR)> selectPhysicalDeviceFunction = nullptr);
	};
}