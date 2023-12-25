#include "lucyvk/vk_pipeline.h"
#include "lucyvk/vk_static.h"
#include "lucyvk/vk_function.h"
#include <fstream>

VkShaderModuleCreateInfo lvk::shader_module_create_info(const char* filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error(std::string("failed to open file! ") + filename);
	}

	size_t size = (size_t)file.tellg();
	char* buffer = new char[size];
	
	file.seekg(0);
	file.read(buffer, size);
	
	file.close();
	
	return {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,
		0,
		size,
		reinterpret_cast<const uint32_t*>(buffer)
	};
}

VkPipelineShaderStageCreateInfo lvk::shader_stage_create_info(VkShaderStageFlagBits flag_bits, VkShaderModule shader_module, const VkSpecializationInfo* specialization) {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		flag_bits,
		shader_module,
		"main",
		specialization
	};
}

VkPipelineShaderStageCreateInfo lvk::shader_stage_create_info(const lvk_shader_module* shader_module, const VkSpecializationInfo* specialization) {
	return shader_stage_create_info(shader_module->_stage, shader_module->_shader_module, specialization);
}

VkPipelineVertexInputStateCreateInfo lvk::vertex_input_state_create_info(const std::vector<VkVertexInputBindingDescription>& binding_description, const std::vector<VkVertexInputAttributeDescription>& attribute_description) {
	return vertex_input_state_create_info(binding_description.data(), binding_description.size(), attribute_description.data(), attribute_description.size());
}


VkPipelineVertexInputStateCreateInfo lvk::vertex_input_state_create_info(const VkVertexInputBindingDescription* binding_description, uint32_t binding_description_count, const VkVertexInputAttributeDescription* attribute_description, uint32_t attribute_description_count) {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		binding_description_count,
		binding_description,
		attribute_description_count,
		attribute_description
	};
}

VkPipelineInputAssemblyStateCreateInfo lvk::input_assembly_state_create_info(const VkPrimitiveTopology topology, bool primitive_restart_enable) {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		topology,
		static_cast<VkBool32>(primitive_restart_enable)
	};
}

VkPipelineRasterizationStateCreateInfo lvk::rasterization_state_create_info(const VkPolygonMode polygon_mode, const VkCullModeFlags cull_mode, const VkFrontFace front_face, float line_width, const bool depth_clamp_enable, const bool discard_rasterizer, const bool depth_bias_enable, const float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor) {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		static_cast<VkBool32>(depth_clamp_enable),
		static_cast<VkBool32>(discard_rasterizer),
		polygon_mode,
		cull_mode,
		front_face,
		static_cast<VkBool32>(depth_bias_enable),
		depth_bias_constant_factor,
		depth_bias_clamp,
		depth_bias_slope_factor,
		line_width
	};
}

VkPipelineRasterizationStateCreateInfo lvk::rasterization_state_create_info(const VkPolygonMode polygon_mode) {
	return rasterization_state_create_info(polygon_mode, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
}

VkPipelineMultisampleStateCreateInfo lvk::multisample_state_create_info(VkSampleCountFlagBits rasterization_sample, bool sample_shading, float min_sample_shading, const VkSampleMask* sample_mask, bool alpha_to_coverage, bool alpha_to_one) {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		rasterization_sample,
		static_cast<VkBool32>(sample_shading),
		min_sample_shading,
		sample_mask,
		static_cast<VkBool32>(alpha_to_coverage),
		static_cast<VkBool32>(alpha_to_one)
	};
}

VkPipelineMultisampleStateCreateInfo lvk::multisample_state_create_info() {
	return multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT, false);
}

VkPipelineColorBlendStateCreateInfo lvk::color_blend_state(const VkPipelineColorBlendAttachmentState* attachments, const uint32_t attachment_count, const bool logic_op_enable, const VkLogicOp logic_op) {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		static_cast<VkBool32>(logic_op_enable),
		logic_op,
		attachment_count,
		attachments,
		{ 0, 0, 0, 0 }
	};
}

VkPipelineColorBlendAttachmentState lvk::color_blend_attachment() {
	// TODO: Better Implementation
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	return colorBlendAttachment;
}
