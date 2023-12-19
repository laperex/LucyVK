#include <lucyvk/Swapchain.h>
#include <lucyvk/Instance.h>
#include <lucyvk/PhysicalDevice.h>
#include <lucyvk/LogicalDevice.h>

lucyvk::Swapchain::Swapchain(lucyvk::Device& device): 
{
	
}

void lucyvk::Swapchain::Initialize() {
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormatKHR(device.swapchainSupportDetails.formats);
	for (const auto& availableFormat: availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
	VkPresentModeKHR presentMode = ChooseSwapPresentModeKHR(device.swapchainSupportDetails.presentModes);
	VkExtent2D extent = ChooseSwapExtent2D(device.swapchainSupportDetails.capabilities);

	uint32_t imageCount = device.swapchainSupportDetails.capabilities.minImageCount + 1;
	if (device.swapchainSupportDetails.capabilities.maxImageCount > 0 && imageCount > device.swapchainSupportDetails.capabilities.maxImageCount) {
		imageCount = device.swapchainSupportDetails.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surfaceKHR;

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

	createInfo.preTransform = device.swapchainSupportDetails.capabilities.currentTransform;
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
