#include "lucyvk/create_info.h"
#include "lucyvk/functions.h"
#include "lucyvk/shaders.h"
#include "lucyvk/command.h"
// #include "lucyvk/synchronization.h"

#include <fstream>
#include <iostream>
#include <string>

VkShaderModuleCreateInfo lvk::info::shader_module(const char* filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error(filename);
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
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.depthTestEnable = depth_test ? VK_TRUE : VK_FALSE,
		.depthWriteEnable = depth_write ? VK_TRUE: VK_FALSE,
		.depthCompareOp = depth_test ? compare_op: VK_COMPARE_OP_ALWAYS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front = {},
		.back = {},
		.minDepthBounds = 0,
		.maxDepthBounds = 1
	};
}

VkPipelineShaderStageCreateInfo lvk::info::shader_stage(VkShaderStageFlagBits flag_bits, VkShaderModule shader_module, const VkSpecializationInfo* specialization) {
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.stage = flag_bits,
		.module = shader_module,
		.pName = "main",
		.pSpecializationInfo = specialization
	};
}

VkPipelineShaderStageCreateInfo lvk::info::shader_stage(const lvk_shader_module& shader_module, const VkSpecializationInfo* specialization) {
	return shader_stage(shader_module._stage, shader_module._shader_module, specialization);
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

VkPipelineVertexInputStateCreateInfo lvk::info::vertex_input_state(const std::vector<VkVertexInputBindingDescription>& binding_description, const std::vector<VkVertexInputAttributeDescription>& attribute_description) {
	return vertex_input_state(binding_description.data(), binding_description.size(), attribute_description.data(), attribute_description.size());
}

VkPipelineInputAssemblyStateCreateInfo lvk::info::input_assembly_state(const VkPrimitiveTopology topology, bool primitive_restart_enable) {
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.topology = topology,
		.primitiveRestartEnable = static_cast<VkBool32>(primitive_restart_enable)
	};
}

VkPipelineRasterizationStateCreateInfo lvk::info::rasterization_state_create_info(const VkPolygonMode polygon_mode, const VkCullModeFlags cull_mode, const VkFrontFace front_face, float line_width, const bool depth_clamp_enable, const bool discard_rasterizer, const bool depth_bias_enable, const float depth_bias_constant_factor, float depth_bias_clamp, float depth_bias_slope_factor) {
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.depthClampEnable = static_cast<VkBool32>(depth_clamp_enable),
		.rasterizerDiscardEnable = static_cast<VkBool32>(discard_rasterizer),
		.polygonMode = polygon_mode,
		.cullMode = cull_mode,
		.frontFace = front_face,
		.depthBiasEnable = static_cast<VkBool32>(depth_bias_enable),
		.depthBiasConstantFactor = depth_bias_constant_factor,
		.depthBiasClamp = depth_bias_clamp,
		.depthBiasSlopeFactor = depth_bias_slope_factor,
		.lineWidth = line_width
	};
}

VkPipelineRasterizationStateCreateInfo lvk::info::rasterization_state(const VkPolygonMode polygon_mode) {
	return rasterization_state_create_info(polygon_mode, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
}

VkPipelineMultisampleStateCreateInfo lvk::info::multisample_state_create_info(VkSampleCountFlagBits rasterization_sample, bool sample_shading, float min_sample_shading, const VkSampleMask* sample_mask, bool alpha_to_coverage, bool alpha_to_one) {
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.rasterizationSamples = rasterization_sample,
		.sampleShadingEnable = static_cast<VkBool32>(sample_shading),
		.minSampleShading = min_sample_shading,
		.pSampleMask = sample_mask,
		.alphaToCoverageEnable = static_cast<VkBool32>(alpha_to_coverage),
		.alphaToOneEnable = static_cast<VkBool32>(alpha_to_one)
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

VkPipelineRenderingCreateInfo lvk::info::rendering(const VkFormat depth_attachment_format, const VkFormat stencil_attachment_format, const VkFormat* color_attachment_formats, const uint32_t color_attachment_formats_count) {
	return {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		
		.colorAttachmentCount = color_attachment_formats_count,
		.pColorAttachmentFormats = color_attachment_formats,

		.depthAttachmentFormat = depth_attachment_format,
		
		.stencilAttachmentFormat = stencil_attachment_format
	};
}

VkImageViewCreateInfo lvk::info::image_view(const VkImage image, VkFormat format, VkImageViewType view_type, VkImageSubresourceRange subresource_range, VkComponentMapping components) {
	return {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
		.image = image,
		.viewType = view_type,
		.format = format,
		.components = components,
		.subresourceRange = subresource_range
	};
}

VkImageViewCreateInfo lvk::info::image_view(const VkImage image, VkFormat format, VkImageAspectFlags aspect_flag, VkImageViewType view_type) {
	return image_view(image, format, view_type, { aspect_flag, 0, 1, 0, 1 }, {});
}

VkSemaphoreSubmitInfo lvk::info::semaphore_submit(VkPipelineStageFlags2 stage_mask, const VkSemaphore semaphore) {
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

VkImageSubresourceRange lvk::info::image_subresource_range(VkImageAspectFlags aspect_mask, uint32_t base_mip_level, uint32_t level_count, uint32_t base_array_layer, uint32_t layer_count) {
	return {
		.aspectMask = aspect_mask,
		.baseMipLevel = base_mip_level,
		.levelCount = level_count,
		.baseArrayLayer = base_array_layer,
		.layerCount = layer_count,
	};
}

VkImageSubresourceRange lvk::info::image_subresource_range(VkImageAspectFlags aspect_mask) {
	return image_subresource_range(aspect_mask, 0, 1, 0, 1);
}

VkImageMemoryBarrier lvk::info::image_memory_barrier(const VkImage image, VkAccessFlags src_access, VkAccessFlags dst_access, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t src_queue_index, uint32_t dst_queue_index, VkImageSubresourceRange subresource_range) {
	return {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,

		.srcAccessMask = src_access,
		.dstAccessMask = dst_access,

		.oldLayout = old_layout,
		.newLayout = new_layout,

		.srcQueueFamilyIndex = src_queue_index,
		.dstQueueFamilyIndex = dst_queue_index,

		.image = image,

		.subresourceRange = subresource_range
	};
}

VkImageMemoryBarrier lvk::info::image_memory_barrier(const VkImage image, VkAccessFlags src_access, VkAccessFlags dst_access, VkImageLayout old_layout, VkImageLayout new_layout, VkImageSubresourceRange subresource_range) {
	return image_memory_barrier(image, src_access, dst_access, old_layout, new_layout, 0, 0, subresource_range);
}

VkCommandBufferSubmitInfo lvk::info::command_buffer_submit(const lvk_command_buffer* command_buffer) {
	return {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,

		.commandBuffer = command_buffer->_command_buffer,
		.deviceMask = 0,
	};
}

VkSubmitInfo lvk::info::submit(const VkCommandBuffer* command_buffers, const uint32_t command_buffer_count, const VkSemaphore* signal_semaphores, const uint32_t signal_semaphores_count, const VkSemaphore* wait_semaphores, const uint32_t wait_semaphore_count) {
	return {
		// .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,

		// .waitSemaphoreCount = wait_semaphore_count,
		// .pWaitSemaphores = wait_semaphores,

		// .commandBufferCount = command_buffer_count,
		// .pCommandBuffers = command_buffers,

		// .signalSemaphoreCount = signal_semaphores_count,
		// .pSignalSemaphores = signal_semaphores,
		
		// .pWaitDstStageMask = 
	};
}
