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
		~Device();

		Swapchain CreateSwapchain(int width, int height);
		CommandPool CreateCommandPool();

		void CreateImage(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) const;

		void WaitIdle();

	private:
		bool Initialize();
		bool Destroy();
	};
}
