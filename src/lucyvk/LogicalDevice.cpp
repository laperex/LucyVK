#include <lucyvk/LogicalDevice.h>
#include <lucyvk/PhysicalDevice.h>
#include <lucyvk/Instance.h>
#include <lucyvk/Swapchain.h>
#include <lucyvk/CommandPool.h>
#include <set>
#include <stdexcept>
#include <util/logger.h>

lucyvk::Device::Device(const Instance& instance, const PhysicalDevice& physicalDevice):
	instance(instance),
	physicalDevice(physicalDevice)
{
	Initialize();
}

lucyvk::Device::~Device()
{
	Destroy();
}

lucyvk::Swapchain lucyvk::Device::CreateSwapchain(int width, int height) {
	return { *this, VkExtent2D { static_cast<uint32_t>(width), static_cast<uint32_t>(height) } };
}

lucyvk::CommandPool lucyvk::Device::CreateCommandPool() {
	return { *this };
}

void lucyvk::Device::CreateImage(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) const {
    if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = physicalDevice.FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    if (vkBindImageMemory(_device, image, imageMemory, 0) != VK_SUCCESS) {
        throw std::runtime_error("failed to bind image memory!");
    }
}

void lucyvk::Device::WaitIdle() {
	vkDeviceWaitIdle(_device);
}


bool lucyvk::Device::Initialize() {
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoArray;
    std::set<uint32_t> uniqueQueueFamilies = { physicalDevice._queueFamilyIndices.graphics.value(), physicalDevice._queueFamilyIndices.present.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamilies) {
		queueCreateInfoArray.push_back({
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			queueFamily,
			1,
			&queuePriority
		});
    }

	VkDeviceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(std::size(queueCreateInfoArray)),
		queueCreateInfoArray.data(),
		static_cast<uint32_t>(std::size(instance.layers)),
		instance.layers.data(),
		static_cast<uint32_t>(std::size(deviceExtensions)),
		deviceExtensions.data(),
		&physicalDevice._features
	};

    if (vkCreateDevice(physicalDevice._physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
        throw std::runtime_error("logical device creation failed!");
    }
	dloggln("Logical Device Created");

    vkGetDeviceQueue(_device, physicalDevice._queueFamilyIndices.graphics.value(), 0, &_graphicsQueue);
	dloggln("Graphics Queue Created");
    vkGetDeviceQueue(_device, physicalDevice._queueFamilyIndices.present.value(), 0, &_presentQueue);
	dloggln("Present Queue Created");

	return true;
}

bool lucyvk::Device::Destroy() {
	vkDestroyDevice(_device, VK_NULL_HANDLE);
	dloggln("Device Destroyed");

	return true;
}
