#pragma once

#include "lvk/types.h"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace lvk {
	VkSurfaceFormatKHR get_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& format_array);
	lvk::swapchain_support_details query_swapchain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
	lvk::queue_family_indices query_queue_family_indices(VkPhysicalDevice physicalDevice, VkSurfaceKHR _surfaceKHR);

	VkPhysicalDevice default_physical_device(const std::vector<VkPhysicalDevice>& physicalDeviceArray, const lvk_instance* instance);

	const char* to_string(const VkBufferUsageFlagBits usage);

	VkPipelineColorBlendAttachmentState color_blend_attachment();
	
	VkDescriptorSetLayoutBinding descriptor_set_layout_binding(uint32_t binding, VkShaderStageFlags shader_stage_flags, VkDescriptorType descriptor_type, uint32_t descriptor_count);
	VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspect_mask);
}
