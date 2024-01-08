#pragma once

#include "vk_types.h"

namespace lvk::config {
	struct graphics_pipeline {
		std::vector<VkPipelineShaderStageCreateInfo> shader_stage_array;

		VkPipelineVertexInputStateCreateInfo vertex_input_state;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_state;
		VkPipelineTessellationStateCreateInfo testallation_state;
		VkPipelineViewportStateCreateInfo viewport_state;
		VkPipelineRasterizationStateCreateInfo rasterization_state;
		VkPipelineMultisampleStateCreateInfo multisample_state;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_state;
		VkPipelineColorBlendStateCreateInfo color_blend_state;
		VkPipelineDynamicStateCreateInfo dynamic_state;

		VkPipelineRenderingCreateInfo dynamic_rendering;
	};
	
	struct instance {
		const char* name;

		std::vector<const char*> layers;
		std::vector<const char*> extensions;

		bool enable_validation_layers;
	};

	struct device {
		const char* name;

		std::vector<const char*> layers;
		std::vector<const char*> extensions;

		bool enable_validation_layers;
	};
	
	struct pipeline_layout {
		const VkPushConstantRange* push_constant_ranges;
		uint32_t push_constant_range_count;
		const VkDescriptorSetLayout* descriptor_set_layouts;
		uint32_t descriptor_set_layout_count;
	};
}