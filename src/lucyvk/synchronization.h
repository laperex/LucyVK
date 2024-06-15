#pragma once

#include "lucyvk/types.h"

#include <vulkan/vulkan_core.h>

// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


// struct lvk_semaphore {
// 	VkSemaphore _semaphore;

// 	const lvk_device* device;
// };


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


// struct lvk_fence {
// 	VkFence _fence;

// 	const lvk_device* device;

// 	VkResult wait(uint64_t timeout = LVK_TIMEOUT) const;
// 	VkResult reset() const;
// };



void lvk_create_semaphore(const VkDevice device, VkSemaphore* semaphore);
void lvk_create_fence(const VkDevice device, VkFence* fence, VkFenceCreateFlags flags = 0);