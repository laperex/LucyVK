#include "lucyvk/vk_info.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_shaders.h"
#include "lucyvk/vk_command.h"
#include <fstream>

VkShaderModuleCreateInfo lvk::info::shader_module(const char* filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error(std::string("failed to open file! ") + filename);
	}

	std::size_t size = (std::size_t)file.tellg();
	char* buffer = new char[size];
	
	file.seekg(0);
	file.read(buffer, size);
	
	file.close();
	
	return {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = size,
		.pCode = reinterpret_cast<const uint32_t*>(buffer)
	};
}

VkPipelineDepthStencilStateCreateInfo lvk::info::depth_stencil_state(bool depth_test, bool depth_write, VkCompareOp compare_op) {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		depth_test ? VK_TRUE : VK_FALSE,
		depth_write ? VK_TRUE: VK_FALSE,
		depth_test ? compare_op: VK_COMPARE_OP_ALWAYS,
		VK_FALSE,
		VK_FALSE,
		{},
		{},
		0,
		1
	};
}

VkPipelineShaderStageCreateInfo lvk::info::shader_stage(VkShaderStageFlagBits flag_bits, VkShaderModule shader_module, const VkSpecializationInfo* specialization) {
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

VkPipelineShaderStageCreateInfo lvk::info::shader_stage(const lvk_shader_module* shader_module, const VkSpecializationInfo* specialization) {
	return shader_stage(shader_module->_stage, shader_module->_shader_module, specialization);
}

VkPipelineVertexInputStateCreateInfo lvk::info::vertex_input_state(const VkVertexInputBindingDescription* binding_description, uint32_t binding_description_count, const VkVertexInputAttributeDescription* attribute_description, uint32_t attribute_description_count) {
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

		.vertexBindingDescriptionCount = binding_description_count,
		.pVertexBindingDescriptions = binding_description,

		.vertexAttributeDescriptionCount = attribute_description_count,
		.pVertexAttributeDescriptions = attribute_description
	};
}

VkPipelineInputAssemblyStateCreateInfo lvk::info::input_assembly_state(const VkPrimitiveTopology topology, bool primitive_restart_enable) {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		topology,
		static_cast<VkBool32>(primitive_restart_enable)
	};
}

VkPipelineRasterizationStateCreateInfo lvk::info::rasterization_state_create_info(const VkPolygonMode polygon_mode, const VkCullModeFlags cull_mode, const VkFrontFace front_face, float line_width, const bool depth_clamp_enable, const bool discard_rasterizer, const bool depth_bias_enable, const float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor) {
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

VkPipelineRasterizationStateCreateInfo lvk::info::rasterization_state(const VkPolygonMode polygon_mode) {
	return rasterization_state_create_info(polygon_mode, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
}

VkPipelineMultisampleStateCreateInfo lvk::info::multisample_state_create_info(VkSampleCountFlagBits rasterization_sample, bool sample_shading, float min_sample_shading, const VkSampleMask* sample_mask, bool alpha_to_coverage, bool alpha_to_one) {
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

VkPipelineMultisampleStateCreateInfo lvk::info::multisample_state() {
	return multisample_state_create_info(VK_SAMPLE_COUNT_1_BIT, false);
}

VkPipelineColorBlendStateCreateInfo lvk::info::color_blend_state(const VkPipelineColorBlendAttachmentState* attachments, const uint32_t attachment_count, const bool logic_op_enable, const VkLogicOp logic_op) {
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,

		.logicOpEnable = static_cast<VkBool32>(logic_op_enable),
		.logicOp = logic_op,

		.attachmentCount = attachment_count,
		.pAttachments = attachments,
	};
}

VkPipelineViewportStateCreateInfo lvk::info::viewport_state(const VkViewport* viewports, const uint32_t viewports_count, const VkRect2D* scissors, const uint32_t scissors_count) noexcept {
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		
		.viewportCount = viewports_count,
		.pViewports = viewports,
		
		.scissorCount = scissors_count,
		.pScissors = scissors
	};
}

VkImageViewCreateInfo lvk::info::image_view(VkImage image, VkFormat format, VkImageViewType view_type, VkImageSubresourceRange subresource_range, VkComponentMapping components) {
	return {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		image,
		view_type,
		format,
		components,
		subresource_range
	};
}

VkImageViewCreateInfo lvk::info::image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flag, VkImageViewType view_type) {
	return image_view(image, format, view_type, { aspect_flag, 0, 1, 0, 1 }, {});
}

VkSemaphoreSubmitInfo lvk::info::semaphore_submit(VkPipelineStageFlags2 stage_mask, VkSemaphore semaphore) {
	return {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
		.semaphore = semaphore,
		.value = 1,
		.stageMask = stage_mask,
		.deviceIndex = 0,
	};
}

VkSubmitInfo2 lvk::info::submit2(const VkCommandBufferSubmitInfo* command_buffer_infos, const uint32_t command_buffer_infos_count, const VkSemaphoreSubmitInfo* signal_semaphore_infos, const uint32_t signal_semaphore_infos_count, const VkSemaphoreSubmitInfo* wait_semaphore_infos, const uint32_t wait_semaphore_infos_count) {
	return {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,

		.waitSemaphoreInfoCount = wait_semaphore_infos_count,
		.pWaitSemaphoreInfos = wait_semaphore_infos,

		.commandBufferInfoCount = command_buffer_infos_count,
		.pCommandBufferInfos = command_buffer_infos,

		.signalSemaphoreInfoCount = signal_semaphore_infos_count,
		.pSignalSemaphoreInfos = signal_semaphore_infos,
	};
}

VkCommandBufferSubmitInfo lvk::info::command_buffer_submit(const lvk_command_buffer* command_buffer) {
	return {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,

		.commandBuffer = command_buffer->_command_buffer,
		.deviceMask = 0,
	};
}
