#pragma once

#include <functional>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <lucyvk/types.h>

namespace lucyvk {
	typedef std::function<VkPhysicalDevice(const std::vector<VkPhysicalDevice>&, const lucyvk::Instance& instance)> SelectPhysicalDeviceFunction;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;

		operator bool() const {
			return graphics.has_value() && present.has_value();
		}

		const bool unique() const {
			return graphics.value() == present.value();
		}
	};

	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct PhysicalDevice {
		VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
		QueueFamilyIndices _queueFamilyIndices;
		SwapchainSupportDetails _swapchainSupportDetails;
		VkPhysicalDeviceFeatures _features;
		VkPhysicalDeviceProperties _properties;

		const Instance& instance;

		PhysicalDevice(const Instance& instance);
		~PhysicalDevice();
		
		bool Initialize(SelectPhysicalDeviceFunction selectPhysicalDeviceFunction = nullptr);
		bool Destroy();
		
		Device CreateLogicalDevice();

		const VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		const uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags propertyFlags);
	};
}