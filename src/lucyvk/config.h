#pragma once

// #include "lucyvk/types.h"
#include <vector>
#include <vulkan/vulkan.h>



namespace lvk::config {
	struct graphics_pipeline {
		std::vector<VkPipelineShaderStageCreateInfo> shader_stage_array;

		VkPipelineVertexInputStateCreateInfo vertex_input_state;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
		VkPipelineTessellationStateCreateInfo testallation_state;
		VkPipelineRasterizationStateCreateInfo rasterization_state;
		VkPipelineMultisampleStateCreateInfo multisample_state;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
		VkPipelineColorBlendStateCreateInfo color_blend_state;
		
		VkPipelineViewportStateCreateInfo viewport_state;
		VkPipelineDynamicStateCreateInfo dynamic_state;

		VkPipelineRenderingCreateInfoKHR rendering_info;
	};
}