#pragma once

#include <lucyvk/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lucyvk {
	struct Swapchain {
		VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
		VkExtent2D _swapchainExtent = { 0, 0 };
		VkSurfaceFormatKHR _swapchainSurfaceFormat;
		VkPresentModeKHR _swapchainPresentMode;

		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;

		const lucyvk::Device& device;

		Swapchain(const lucyvk::Device& device, VkExtent2D windowExtent);
		~Swapchain();
		
		ImageView CreateImageView();

	private:
		bool Initialize();
		bool Destroy();
	};
}