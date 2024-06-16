#pragma once

#include "lucyvk/config.h"
#include "lucyvk/types.h"
#include <vulkan/vulkan_core.h>


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


struct lvk_pipeline_layout {
	VkPipelineLayout _pipeline_layout;
};


// |--------------------------------------------------
// ----------------> PIPELINE
// |--------------------------------------------------


struct lvk_pipeline {
	VkPipeline _pipeline;
	
	VkPipelineBindPoint type;
};