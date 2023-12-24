#pragma once

#include "lucyvk/vk_static.h"
#include <vulkan/vulkan_core.h>

namespace lvk {
	struct pipeline_config {
		VkPipelineVertexInputStateCreateInfo vertex_input_state;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
		VkPipelineTessellationStateCreateInfo testallation_state;
		VkPipelineViewportStateCreateInfo viewport_state;
		VkPipelineRasterizationStateCreateInfo rasterization_state;
		VkPipelineMultisampleStateCreateInfo multisample_state;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
		VkPipelineColorBlendStateCreateInfo color_blend_state;
		VkPipelineDynamicStateCreateInfo dynamic_state;
	};

	VkShaderModuleCreateInfo shader_module_create_info(const char* filename);

	VkPipelineShaderStageCreateInfo shader_stage_create_info(VkShaderStageFlagBits flag, VkShaderModule shader_module, const VkSpecializationInfo* specialization = nullptr);
	VkPipelineShaderStageCreateInfo shader_stage_create_info(const lvk_shader_module* shader_module, const VkSpecializationInfo* specialization = nullptr);
	
	
}