#include <lucyvk/Swapchain.h>
#include <lucyvk/Instance.h>
#include <lucyvk/PhysicalDevice.h>
#include <lucyvk/LogicalDevice.h>

lucyvk::Swapchain::Swapchain(lucyvk::Device& device, VkExtent2D windowExtent): device(device)
{
	
}

void lucyvk::Swapchain::Initialize() {
	const auto& presentModes = device.physicalDevice._swapchainSupportDetails.presentModes;
	const auto& capabilities = device.physicalDevice._swapchainSupportDetails.capabilities;
	
	uint32_t imageCount = (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) ? capabilities.maxImageCount: capabilities.minImageCount + 1;

	VkExtent2D extent = (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) ?
		capabilities.currentExtent:
		VkExtent2D {
			std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, _swapchainExtent.width)),
			std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, _swapchainExtent.height))
		};

	VkSurfaceFormatKHR surfaceFormat = device.physicalDevice._swapchainSupportDetails.formats[0];
	for (const auto& availableFormat: device.physicalDevice._swapchainSupportDetails.formats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surfaceFormat = availableFormat;
			break;
		}
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	// std::cout << "Present mode: V-Sync" << std::endl;
	for (const auto& availablePresentMode: presentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			// std::cout << "Swapchain Present Mode: Mailbox\n";

			presentMode = availablePresentMode;
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
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		createInfo.imageSharingMode;
		createInfo.queueFamilyIndexCount;
		createInfo.pQueueFamilyIndices;
		createInfo.preTransform;
		createInfo.compositeAlpha;
		createInfo.presentMode;
		createInfo.clipped;
		createInfo.oldSwapchain;
	}

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = {
		device.queueFamilyIndices.graphicsFamily.value(),
		device.queueFamilyIndices.presentFamily.value()
	};

	if (device.queueFamilyIndices.graphicsFamily.value() != device.queueFamilyIndices.presentFamily.value()) {
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

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device.device(), &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
		throw std::runtime_error("FAILED TO CREATE SWAPCHAIN!");
	}

	vkGetSwapchainImagesKHR(device.device(), swapchain, &imageCount, nullptr);
	_swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device.device(), swapchain, &imageCount, _swapchainImages.data());
	
	swapchainImageFormat = surfaceFormat.format;
}
