#include "lvk/synchronization.h"
#include "lvk/logical_device.h"
#include "util/logger.h"



// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


lvk_semaphore lvk_device::init_semaphore() {
	lvk_semaphore semaphore = {
		VK_NULL_HANDLE,
		this
	};

	VkSemaphoreCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		VK_NULL_HANDLE,
		0
	};
	
	if (vkCreateSemaphore(_device, &createInfo, VK_NULL_HANDLE, &semaphore._semaphore) != VK_SUCCESS) {
		throw std::runtime_error("semaphore creation failed");
	}
	dloggln("Semaphore Created");

	deletion_queue.push([=]{
		vkDestroySemaphore(_device, semaphore._semaphore, VK_NULL_HANDLE);
		dloggln("Semaphore Destroyed");
	});
	
	return semaphore;
}


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


lvk_fence lvk_device::init_fence(VkFenceCreateFlags flags) {
	lvk_fence fence = {
		VK_NULL_HANDLE,
		this
	};

	VkFenceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		VK_NULL_HANDLE,
		flags
	};

	if (vkCreateFence(_device, &createInfo, VK_NULL_HANDLE, &fence._fence) != VK_SUCCESS) {
		throw std::runtime_error("fence creation failed");
	}
	dloggln("Fence Created");
	
	deletion_queue.push([=]{
		vkDestroyFence(_device, fence._fence, VK_NULL_HANDLE);
		dloggln("Fence Destroyed");
	});

	return fence;
}

VkResult lvk_fence::wait(uint64_t timeout) const {
	return vkWaitForFences(device->_device, 1, &_fence, false, timeout);
}

VkResult lvk_fence::reset() const {
	return vkResetFences(device->_device, 1, &_fence);
}