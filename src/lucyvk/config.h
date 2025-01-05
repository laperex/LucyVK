#pragma once

// #include "lucyvk/types.h"
#include <vector>
#include <vulkan/vulkan.h>



namespace lvk::config {
	struct compute_pipeline {
		
	};

	struct graphics_pipeline {
		std::vector<VkPipelineShaderStageCreateInfo> shader_stage_array;

		VkPipelineVertexInputStateCreateInfo vertex_input_state;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
		VkPipelineTessellationStateCreateInfo testallation_state;
		VkPipelineRasterizationStateCreateInfo rasterization_state;
		VkPipelineMultisampleStateCreateInfo multisample_state;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
		
		VkPipelineColorBlendStateCreateInfo color_blend_state;
		std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments;
		
		VkPipelineViewportStateCreateInfo viewport_state;
		std::vector<VkDynamicState> dynamic_state_array;

		VkPipelineRenderingCreateInfoKHR rendering_info;
	};
	
	struct graphics_pipeline_properties {
		std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
	};
}