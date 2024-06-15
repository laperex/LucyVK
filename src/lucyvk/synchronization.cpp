#include "lucyvk/synchronization.h"
#include "lucyvk/device.h"
#include "lucyio/logger.h"


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


// VkSemaphore lvk_device::create_semaphore() {
// 	VkSemaphoreCreateInfo create_info = {
// 		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
// 		.pNext = VK_NULL_HANDLE,
// 		.flags = 0,
// 	};

// 	// VkSemaphore semaphore = VK_NULL_HANDLE;

// 	if (vkCreateSemaphore(_device, &create_info, VK_NULL_HANDLE, &semaphore) != VK_SUCCESS) {
// 		throw std::runtime_error("semaphore creation failed");
// 	}
// 	dloggln("Semaphore Created");

// 	deletion_queue.push([=]{
// 		vkDestroySemaphore(_device, semaphore, VK_NULL_HANDLE);
// 		dloggln("Semaphore Destroyed");
// 	});
	
// 	return semaphore;
// }


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


void lvk_create_semaphore(const VkDevice device, VkSemaphore* semaphore) {
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


template<> template<> void lvk_instance_destructor<VkDevice>::push(VkSemaphore semaphore) {
	deletion_queue.push_back([=] (const VkDevice device) {
		vkDestroySemaphore(device, semaphore, VK_NULL_HANDLE);
		dloggln("Semaphore Destroyed");
	});
}


void lvk_create_fence(const VkDevice device, VkFence* fence, VkFenceCreateFlags flags) {
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


template<> template<> void lvk_instance_destructor<VkDevice>::push(VkFence fence) {
	deletion_queue.push_back([=] (const VkDevice device) {
		vkDestroyFence(device, fence, VK_NULL_HANDLE);
		dloggln("Fence Destroyed");
	});
}