#pragma once

#include <lucyvk/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lucyvk {
	struct LogicalDevice {
		VkDevice _device;

		VkQueue _graphicsQueue;
		VkQueue _presentQueue;

		const Instance& instance;
		const PhysicalDevice& physicalDevice;

		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		LogicalDevice(const Instance& instance, const PhysicalDevice& physicalDevice);

		bool Initialize();
	};
}
