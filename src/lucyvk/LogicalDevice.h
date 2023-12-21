#pragma once

#include <lucyvk/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lucyvk {
	struct Device {
		VkDevice _device = VK_NULL_HANDLE;

		VkQueue _graphicsQueue = VK_NULL_HANDLE;
		VkQueue _presentQueue = VK_NULL_HANDLE;

		const Instance& instance;
		const PhysicalDevice& physicalDevice;

		Device(const PhysicalDevice& physicalDevice, const std::vector<const char*>& deviceExtensions, const std::vector<const char*>& layers);
		~Device();

		Swapchain CreateSwapchain(int width, int height);
		CommandPool CreateCommandPool();

		void WaitIdle();
	};
}
