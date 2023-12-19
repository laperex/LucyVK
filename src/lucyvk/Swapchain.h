#pragma once

#include <lucyvk/types.h>
#include <vulkan/vulkan_core.h>

namespace lucyvk {
	struct Swapchain {
		const Device& device;
		
		VkExtent2D _swapchainExtent;
		
		Swapchain(lucyvk::Device& device, VkExtent2D windowExtent);

		void Initialize();
	};
}