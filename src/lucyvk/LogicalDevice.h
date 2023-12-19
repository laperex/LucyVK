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

		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		Device(const Instance& instance, const PhysicalDevice& physicalDevice);

		Swapchain CreateSwapchain(int width, int height);
		
		bool Initialize();
		bool Destroy();
		
		void WaitIdle();
	};
}
