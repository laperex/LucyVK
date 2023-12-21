#pragma once

#include <lucyvk/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lucyvk {
	enum ATTACHMENT {
		DEPTH,
		COLOR,
	};

	struct ImageView {
		const Swapchain& swapchain;

		std::vector<VkImage> _images;
		std::vector<VkImageView> _imageViewArray;
		std::vector<VkDeviceMemory> _imageMemory;
		
		ImageView(const lucyvk::Swapchain& swapchain);
		~ImageView();
		
		bool Initialize();
		bool Destroy();
	};
}