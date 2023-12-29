#pragma once

#include "vk_types.h"

namespace lvk::config {
	struct graphics_pipeline {
		std::vector<VkPipelineShaderStageCreateInfo> shader_stage_array;

		VkPipelineVertexInputStateCreateInfo vertex_input_state;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
		// VkPipelineTessellationStateCreateInfo testallation_state;
		// VkPipelineViewportStateCreateInfo viewport_state;
		VkPipelineRasterizationStateCreateInfo rasterization_state;
		VkPipelineMultisampleStateCreateInfo multisample_state;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
		// VkPipelineColorBlendStateCreateInfo color_blend_state;
		// VkPipelineDynamicStateCreateInfo dynamic_state;
		
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		
		VkViewport viewport;
		VkRect2D scissor;
	};
}