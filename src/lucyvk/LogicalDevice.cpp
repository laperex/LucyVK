#include <lucyvk/LogicalDevice.h>
#include <lucyvk/PhysicalDevice.h>
#include <lucyvk/Instance.h>
#include <lucyvk/Swapchain.h>
#include <set>
#include <stdexcept>
#include <util/logger.h>

lucyvk::Device::Device(const Instance& instance, const PhysicalDevice& physicalDevice):
	instance(instance),
	physicalDevice(physicalDevice)
{
	
}

lucyvk::Swapchain lucyvk::Device::CreateSwapchain(int width, int height) {
	return { *this, VkExtent2D { static_cast<uint32_t>(width), static_cast<uint32_t>(height) } };
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

	return true;
}

void lucyvk::Device::WaitIdle() {
	
}