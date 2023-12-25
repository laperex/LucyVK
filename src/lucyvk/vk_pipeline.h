#pragma once

#include "lucyvk/vk_types.h"
#include <vulkan/vulkan_core.h>

namespace lvk {
	VkShaderModuleCreateInfo shader_module_create_info(const char* filename);

	VkPipelineShaderStageCreateInfo shader_stage_create_info(VkShaderStageFlagBits flag, VkShaderModule shader_module, const VkSpecializationInfo* specialization = nullptr);
	VkPipelineShaderStageCreateInfo shader_stage_create_info(const lvk_shader_module* shader_module, const VkSpecializationInfo* specialization = nullptr);
	
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info(const std::vector<VkVertexInputBindingDescription>& binding_description, const std::vector<VkVertexInputAttributeDescription>& attribute_description);
	VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info(const VkVertexInputBindingDescription* binding_description, uint32_t binding_description_count, const VkVertexInputAttributeDescription* attribute_description, uint32_t attribute_description_count);
	
	VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info(const VkPrimitiveTopology topology, bool primitive_restart_enable);
	
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(const VkPolygonMode polygon_mode, const VkCullModeFlags cull_mode, const VkFrontFace front_face, float line_width = 1.0, const bool depth_clamp_enable = false, const bool discard_rasterizer = false, const bool depth_bias_enable = false, const float depth_bias_constant_factor = 0, float depth_bias_clamp = 0, float depth_bias_slope_factor = 0);
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(const VkPolygonMode polygon_mode);
	
	VkPipelineMultisampleStateCreateInfo multisample_state_create_info(VkSampleCountFlagBits rasterization_sample, bool sample_shading, float min_sample_shading = 1.0, const VkSampleMask* sample_mask = VK_NULL_HANDLE, bool alpha_to_coverage = false, bool alpha_to_one = false);
	VkPipelineMultisampleStateCreateInfo multisample_state_create_info();

	VkPipelineColorBlendStateCreateInfo color_blend_state(const VkPipelineColorBlendAttachmentState* attachments, const uint32_t attachment_count, const bool logic_op_enable = false, const VkLogicOp logic_op = VK_LOGIC_OP_COPY);

	VkPipelineColorBlendAttachmentState color_blend_attachment();
	// VkPipelineColorBlendAttachmentStatecolor_blend_attachment_state
}