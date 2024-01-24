#include "lvk/pipeline.h"
#include "lvk/logical_device.h"
#include "lvk/render_pass.h"
#include "util/logger.h"
#include <stdexcept>




// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


lvk_pipeline_layout lvk_device::init_pipeline_layout(const VkPushConstantRange* push_constant_ranges, uint32_t push_constant_range_count, const VkDescriptorSetLayout* descriptor_set_layouts, uint32_t descriptor_set_layout_count) {
	lvk_pipeline_layout pipeline_layout = {
		._pipeline_layout = VK_NULL_HANDLE,
		.device = this,
		.deletion_queue = &deletion_queue,
	};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,

		.setLayoutCount = descriptor_set_layout_count,
		.pSetLayouts = descriptor_set_layouts,

		.pushConstantRangeCount = push_constant_range_count,
		.pPushConstantRanges = push_constant_ranges
	};
	
	if (vkCreatePipelineLayout(this->_device, &pipeline_layout_create_info, VK_NULL_HANDLE, &pipeline_layout._pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("pipeline layout creation failed!");
	}
	dloggln("Pipeline Layout Created");
	
	deletion_queue.push([=]{
		vkDestroyPipelineLayout(_device, pipeline_layout._pipeline_layout, VK_NULL_HANDLE);
		dloggln("Pipeline Layout Destroyed");
	});
	
	return pipeline_layout;
}

lvk_pipeline lvk_pipeline_layout::init_compute_pipeline(const VkPipelineShaderStageCreateInfo stage_info) {
	lvk_pipeline pipeline = {
		._pipeline = VK_NULL_HANDLE,
		.pipeline_layout = this,
		.render_pass = VK_NULL_HANDLE,
		.device = device,
		.type = VK_PIPELINE_BIND_POINT_COMPUTE,
		.deletion_queue = deletion_queue,
	};

	VkComputePipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = stage_info,
		.layout = _pipeline_layout,
	};

	if (vkCreateComputePipelines(device->_device, VK_NULL_HANDLE, 1, &pipeline_info, VK_NULL_HANDLE, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("compute pipeline creation failed!");
	}
	dloggln("Compute Pipeline Created");
	
	deletion_queue->push([=]{
		vkDestroyPipeline(device->_device, pipeline._pipeline, VK_NULL_HANDLE);
		dloggln("Compute Pipeline Destroyed");
	});

	return pipeline;
}

lvk_pipeline lvk_pipeline_layout::init_graphics_pipeline(const lvk::config::graphics_pipeline* config, const lvk_render_pass* render_pass) {
	lvk_pipeline pipeline = {
		._pipeline = VK_NULL_HANDLE,
		.pipeline_layout = this,
		.render_pass = render_pass,
		.device = device,
		.type = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.deletion_queue = deletion_queue,
	};
	
	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

		.pNext = (config->rendering_info.sType == VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO) ? &config->rendering_info: VK_NULL_HANDLE,

		.stageCount = static_cast<uint32_t>(config->shader_stage_array.size()),
		.pStages = config->shader_stage_array.data(),

		.pVertexInputState = &config->vertex_input_state,
		.pInputAssemblyState = &config->input_assembly_state,
		.pViewportState = &config->viewport_state,
		.pRasterizationState = &config->rasterization_state,
		.pMultisampleState = &config->multisample_state,
		.pDepthStencilState = &config->depth_stencil_state,
		.pColorBlendState = &config->color_blend_state,
		.layout = this->_pipeline_layout,
		.renderPass = (render_pass != VK_NULL_HANDLE) ? render_pass->_render_pass: VK_NULL_HANDLE,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
	};

	if (vkCreateGraphicsPipelines(device->_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("graphics pipeline creation failed!");
	}
	dloggln("Graphics Pipeline Created");
	
	deletion_queue->push([=]{
		vkDestroyPipeline(device->_device, pipeline._pipeline, VK_NULL_HANDLE);
		dloggln("Graphics Pipeline Destroyed");
	});
	
	return pipeline;
}
