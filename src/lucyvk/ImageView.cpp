#include "util/logger.h"
#include <cassert>
#include <lucyvk/ImageView.h>
#include <lucyvk/Swapchain.h>
#include <lucyvk/LogicalDevice.h>
#include <lucyvk/PhysicalDevice.h>
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
	assert(vkGetSwapchainImagesKHR(swapchain.device._device, swapchain._swapchain, &imageCount, _images.data()) == VK_SUCCESS);

	for (size_t i = 0; i < _images.size(); i++) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		viewInfo.image = _images[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = swapchain._swapchainSurfaceFormat.format;

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		
		// VkImageCreateInfo imageInfo = {};
		// imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		// imageInfo.imageType = VK_IMAGE_TYPE_2D;
		// imageInfo.extent.width = swapchain._swapchainExtent.width;
		// imageInfo.extent.height = swapchain._swapchainExtent.height;
		// imageInfo.extent.depth = 1;
		// imageInfo.mipLevels = 1;
		// imageInfo.arrayLayers = 1;

		// imageInfo.format = swapchain.device.physicalDevice.FindSupportedFormat(
		// 	{
		// 		VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT
		// 	},
		// 	VK_IMAGE_TILING_OPTIMAL,
		// 	VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
		// );

		// imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		// imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		// imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		// imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		// imageInfo.flags = 0;

		// swapchain.device.CreateImage(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _images[i], _imageMemory[i]);

		// VkImageViewCreateInfo viewInfo{};
		// viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		// viewInfo.image = depthImages[i];
		// viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		// viewInfo.format = depthFormat;
		// viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		// viewInfo.subresourceRange.baseMipLevel = 0;
		// viewInfo.subresourceRange.levelCount = 1;
		// viewInfo.subresourceRange.baseArrayLayer = 0;
		// viewInfo.subresourceRange.layerCount = 1;

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

