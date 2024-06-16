#pragma once

#include "lucyvk/types.h"
#include <vulkan/vulkan_core.h>


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


struct lvk_semaphore {
	VkSemaphore _semaphore;
};


// // |--------------------------------------------------
// // ----------------> FENCE
// // |--------------------------------------------------


struct lvk_fence {
	VkFence _fence;
};



// void lvk_create_semaphore(const VkDevice device, VkSemaphore* semaphore);
void lvk_create_fence(const VkDevice device, VkFence* fence, VkFenceCreateFlags flags = 0);