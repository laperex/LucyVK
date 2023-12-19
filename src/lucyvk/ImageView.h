#pragma once

#include <lucyvk/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lucyvk {
	struct ImageView {
		const Swapchain& swapchain;

		std::vector<VkImage> _images;
		std::vector<VkImageView> _imageViews;
		
		ImageView(const lucyvk::Swapchain& swapchain);
		
		bool Initialize();
	};
}