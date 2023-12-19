#include "util/logger.h"
#include <lucyvk/Swapchain.h>
#include <lucyvk/Instance.h>
#include <lucyvk/PhysicalDevice.h>
#include <lucyvk/LogicalDevice.h>
#include <stdexcept>

lucyvk::Swapchain::Swapchain(const lucyvk::Device& device, VkExtent2D windowExtent): device(device)
{
	
}

lucyvk::Swapchain::~Swapchain()
{
	
}

bool lucyvk::Swapchain::Initialize() {
	const auto& presentModes = device.physicalDevice._swapchainSupportDetails.presentModes;
	const auto& capabilities = device.physicalDevice._swapchainSupportDetails.capabilities;


	uint32_t imageCount = (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) ? capabilities.maxImageCount: capabilities.minImageCount + 1;

	VkExtent2D extent = (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) ?
		capabilities.currentExtent:
		VkExtent2D {
			std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, _swapchainExtent.width)),
			std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, _swapchainExtent.height))
		};


	_swapchainSurfaceFormat = device.physicalDevice._swapchainSupportDetails.formats[0];
	for (const auto& availableFormat: device.physicalDevice._swapchainSupportDetails.formats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			_swapchainSurfaceFormat = availableFormat;
			break;
		}
	}


	// TODO: support for more presentMode types
	// * VK_PRESENT_MODE_FIFO_KHR = V-Sync
	// * VK_PRESENT_MODE_MAILBOX_KHR = Mailbox

	_swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& availablePresentMode: presentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			_swapchainPresentMode = availablePresentMode;
			break;
		}
	}


	VkSwapchainCreateInfoKHR createInfo = {};
	{
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = device.physicalDevice.instance._surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = _swapchainSurfaceFormat.format;
		createInfo.imageColorSpace = _swapchainSurfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// TODO: better approach
		uint32_t queueFamilyIndices[] = {
			device.physicalDevice._queueFamilyIndices.graphics.value(),
			device.physicalDevice._queueFamilyIndices.present.value()
		};

		if (device.physicalDevice._queueFamilyIndices.unique()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = _swapchainPresentMode;

		// TODO: user defined clipping state
		createInfo.clipped = VK_TRUE;

		// TODO: remains to be tested
		createInfo.oldSwapchain = _swapchain;


		if (vkCreateSwapchainKHR(device._device, &createInfo, VK_NULL_HANDLE, &_swapchain) != VK_SUCCESS) {
			throw std::runtime_error("FAILED TO CREATE SWAPCHAIN!");
		}
		dloggln("Created Swapchain");
	}

	return true;
}

bool lucyvk::Swapchain::Destroy() {
	vkDestroySwapchainKHR(device._device, _swapchain, VK_NULL_HANDLE);
	dloggln("Swapchain Destroyed");

	return true;
}
