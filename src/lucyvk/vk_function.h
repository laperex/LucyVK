#pragma once

#include <vulkan/vulkan_core.h>
namespace lvk {
	VkShaderModule load_shader_module(const char* filename);
}