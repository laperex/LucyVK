#pragma once

#include <lucyvk/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lucyvk {
	struct Swapchain {
		const Device& device;
		
		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
		VkExtent2D _swapchainExtent;
		VkSurfaceFormatKHR _swapchainSurfaceFormat;
		VkPresentModeKHR _swapchainPresentMode;

		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;

		Swapchain(lucyvk::Device& device, VkExtent2D windowExtent);

		bool Initialize();
		bool Destroy();
	};
}