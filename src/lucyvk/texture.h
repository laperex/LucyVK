#pragma once

#include "lucyvk/types.h"
#include <vulkan/vulkan_core.h>

struct lvk_sampler {
	VkSampler _sampler;
	
	const lvk_device* device;
};