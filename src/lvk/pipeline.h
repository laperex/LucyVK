#pragma once

#include "lvk/config.h"
#include "lvk/types.h"
#include <vulkan/vulkan_core.h>


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


struct lvk_pipeline_layout {
	VkPipelineLayout _pipeline_layout;

	const lvk_device* device;
	
	lvk::deletion_queue* deletion_queue;

	lvk_pipeline init_graphics_pipeline(const lvk::config::graphics_pipeline* config, const lvk_render_pass* render_pass = VK_NULL_HANDLE);

	lvk_pipeline init_compute_pipeline(const VkPipelineShaderStageCreateInfo stage_info);
};


// |--------------------------------------------------
// ----------------> PIPELINE
// |--------------------------------------------------


struct lvk_pipeline {
	VkPipeline _pipeline;
	
	const lvk_pipeline_layout* pipeline_layout;
	const lvk_render_pass* render_pass;
	const lvk_device* device;
	
	VkPipelineBindPoint type;
	
	lvk::deletion_queue* deletion_queue;
};