#include "util/logger.h"
#include <lucyvk/ImageView.h>
#include <lucyvk/Swapchain.h>
#include <lucyvk/LogicalDevice.h>
#include <stdexcept>

lucyvk::ImageView::ImageView(const lucyvk::Swapchain& swapchain):
	swapchain(swapchain)
{
	Initialize();
}

lucyvk::ImageView::~ImageView()
{
	Destroy();
}

bool lucyvk::ImageView::Initialize() {
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(swapchain.device._device, swapchain._swapchain, &imageCount, nullptr);
	_images.resize(imageCount);
	_imageViewArray.resize(_images.size());
	vkGetSwapchainImagesKHR(swapchain.device._device, swapchain._swapchain, &imageCount, _images.data());

	for (size_t i = 0; i < _images.size(); i++) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		viewInfo.image = _images[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = swapchain._swapchainSurfaceFormat.format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(swapchain.device._device, &viewInfo, VK_NULL_HANDLE, &_imageViewArray[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
	}
	
	return true;
}

bool lucyvk::ImageView::Destroy() {
	for (auto& imageView: _imageViewArray) {
		vkDestroyImageView(swapchain.device._device, imageView, nullptr);
	}
	dloggln("ImageViews Destroyed");

	return true;
}

