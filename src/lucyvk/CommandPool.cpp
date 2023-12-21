#include "util/logger.h"
#include <lucyvk/CommandPool.h>
#include <lucyvk/LogicalDevice.h>
#include <lucyvk/PhysicalDevice.h>
#include <stdexcept>

lucyvk::CommandPool::CommandPool(const Device& device):
	device(device)
{
    Initialize();
}

lucyvk::CommandPool::~CommandPool()
{
	Destroy();
}

bool lucyvk::CommandPool::Initialize() {
	VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = device.physicalDevice._queueFamilyIndices.graphics.value();
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    if (vkCreateCommandPool(device._device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

	return true;
}

bool lucyvk::CommandPool::Destroy() {
	vkDestroyCommandPool(device._device, _commandPool, VK_NULL_HANDLE);
	dloggln("Command Pool Destroyed");
	
	return true;
}
