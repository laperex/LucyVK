#include "lucyvk/vk_swapchain.h"
#include "lucyvk/vk_device.h"
#include "lucyvk/vk_physical_device.h"
#include "lucyvk/vk_instance.h"
#include "util/logger.h"



// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


lvk_swapchain lvk_device::init_swapchain(uint32_t width, uint32_t height, VkImageUsageFlags image_usage_flags, VkSurfaceFormatKHR surface_format) {
	const auto& capabilities = physical_device->_swapchain_support_details.capabilities;

	lvk_swapchain swapchain = {
		._swapchain = VK_NULL_HANDLE,
		._extent = VkExtent2D { width, height },
		._surface_format = surface_format,
		._present_mode = VK_PRESENT_MODE_FIFO_KHR,
		._image_usage = image_usage_flags,
		.device = this,
		.physical_device = physical_device,
		.instance = instance
	};

	// TODO: support for more presentMode types
	// * VK_PRESENT_MODE_IMMEDIATE_KHR = Not suggested causes tearing
	// * VK_PRESENT_MODE_MAILBOX_KHR = Mailbox
	// * VK_PRESENT_MODE_FIFO_KHR = V-sync
	// * VK_PRESENT_MODE_FIFO_RELAXED_KHR
	// * VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR
	// * VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR

	for (const auto& availablePresentMode: physical_device->_swapchain_support_details.present_modes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			swapchain._present_mode = availablePresentMode;
			break;
		}
		if (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
			swapchain._present_mode = availablePresentMode;
			break;
		}
	}
	
	swapchain.recreate(width, height);
	
	deletion_queue.push([=]{
		vkDestroySwapchainKHR(_device, swapchain._swapchain, VK_NULL_HANDLE);
		dloggln("Swapchain Destroyed");

		for (int i = 0; i < swapchain._image_count; i++) {
			vkDestroyImageView(_device, swapchain._image_views[i], VK_NULL_HANDLE);
		}
		dloggln("ImageViews Destroyed");
		
		delete [] swapchain._image_views;
		delete [] swapchain._images;
	});

	return swapchain;
}

bool lvk_swapchain::recreate(const uint32_t width, const uint32_t height) {
	if (_image_count) {
		for (int i = 0; i < _image_count; i++) {
			vkDestroyImageView(device->_device, _image_views[i], VK_NULL_HANDLE);
		}
		dloggln("ImageViews Destroyed");
	}

	if (_swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device->_device, _swapchain, VK_NULL_HANDLE);
		dloggln("Swapchain Destroyed");
	}

	this->_extent.width = width;
	this->_extent.height = height;

	const auto& present_modes = physical_device->_swapchain_support_details.present_modes;
	const auto& capabilities = physical_device->_swapchain_support_details.capabilities;

	VkSwapchainCreateInfoKHR createInfo = {};
	{
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = instance->_surface;

		// TF ???
		createInfo.minImageCount = (capabilities.maxImageCount > 0 && capabilities.minImageCount + 1 > capabilities.maxImageCount) ? capabilities.maxImageCount: capabilities.minImageCount + 1;

		createInfo.imageFormat = this->_surface_format.format;
		createInfo.imageColorSpace = this->_surface_format.colorSpace;
		createInfo.imageExtent = (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) ?
			capabilities.currentExtent:
			VkExtent2D {
				std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, this->_extent.width)),
				std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, this->_extent.height))
			};
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = _image_usage;

		// TODO: better approach
		uint32_t queueFamilyIndices[] = {
			physical_device->_queue_family_indices.graphics.value(),
			physical_device->_queue_family_indices.present.value()
		};

		// TODO: Sharing Mode is always exclusive in lvk_buffer. Therefore only one queue is possible
		if (physical_device->_queue_family_indices.present == physical_device->_queue_family_indices.graphics) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		} else {
			throw std::runtime_error("VK_SHARING_MODE_CONCURRENT is not implemented yet");
			
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = std::size(queueFamilyIndices);
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = this->_present_mode;

		// TODO: user defined clipping state
		createInfo.clipped = VK_TRUE;

		// TODO: remains to be tested
		createInfo.oldSwapchain = VK_NULL_HANDLE;


		if (vkCreateSwapchainKHR(this->device->_device, &createInfo, VK_NULL_HANDLE, &this->_swapchain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swapchain!");
		}
		dloggln("Created Swapchain");
	}
	
	// ImageViews
	
	vkGetSwapchainImagesKHR(this->device->_device, this->_swapchain, &_image_count, VK_NULL_HANDLE);
	_images = new VkImage[_image_count];
	_image_views = new VkImageView[_image_count];
	vkGetSwapchainImagesKHR(this->device->_device, this->_swapchain, &_image_count, _images);

	for (size_t i = 0; i < _image_count; i++) {
		VkImageViewCreateInfo viewInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = _images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = this->_surface_format.format,

			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		if (vkCreateImageView(this->device->_device, &viewInfo, VK_NULL_HANDLE, &this->_image_views[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
	}
	dloggln("ImageViews Created");

	return true;
}

VkResult lvk_swapchain::acquire_next_image(uint32_t* index, VkSemaphore semaphore, VkFence fence, const uint64_t timeout) {
	return vkAcquireNextImageKHR(device->_device, _swapchain, timeout, semaphore, fence, index);
}