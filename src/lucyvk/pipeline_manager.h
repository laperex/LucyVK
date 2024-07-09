#pragma once

#include "lucyvk/handles.h"


struct lvk_material_info {
	lvk_pipeline* pipeline;
};


struct lvk_pipeline_manager {
	lvk_device* device;

	// std::vector<>

	void add_custom_pipeline(const char* name, VkGraphicsPipelineCreateInfo create_info);
};