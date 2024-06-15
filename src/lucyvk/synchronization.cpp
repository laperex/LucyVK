#include "lucyvk/synchronization.h"
#include "lucyvk/device.h"
#include "lucyio/logger.h"


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


// lvk_fence lvk_device::init_fence(VkFenceCreateFlags flags) {
// 	lvk_fence fence = {
// 		VK_NULL_HANDLE,
// 		this
// 	};

// 	VkFenceCreateInfo createInfo = {
// 		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
// 		VK_NULL_HANDLE,
// 		flags
// 	};

// 	if (vkCreateFence(_device, &createInfo, VK_NULL_HANDLE, &fence._fence) != VK_SUCCESS) {
// 		throw std::runtime_error("fence creation failed");
// 	}
// 	dloggln("Fence Created");
	
// 	deletion_queue.push([=]{
// 		vkDestroyFence(_device, fence._fence, VK_NULL_HANDLE);
// 		dloggln("Fence Destroyed");
// 	});

// 	return fence;
// }

// VkResult lvk_fence::wait(uint64_t timeout) const {
// 	return vkWaitForFences(device->_device, 1, &_fence, false, timeout);
// }

// VkResult lvk_fence::reset() const {
// 	return vkResetFences(device->_device, 1, &_fence);
// }


void lvk_create_semaphore(VkDevice device, VkSemaphore* semaphore) {
	VkSemaphoreCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
	};

	if (vkCreateSemaphore(device, &create_info, VK_NULL_HANDLE, semaphore) != VK_SUCCESS) {
		throw std::runtime_error("semaphore creation failed");
	}
	dloggln("Semaphore Created");
}


void lvk_create_fence(VkDevice device, VkFence* fence, VkFenceCreateFlags flags) {
	VkFenceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = flags
	};

	if (vkCreateFence(device, &create_info, VK_NULL_HANDLE, fence) != VK_SUCCESS) {
		throw std::runtime_error("fence creation failed");
	}
	dloggln("Fence Created");
}
