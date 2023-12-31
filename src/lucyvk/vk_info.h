#pragma once

#include "lucyvk/vk_types.h"
#include <vulkan/vulkan_core.h>

namespace lvk::info {
	VkShaderModuleCreateInfo shader_module(const char* filename);
	
	VkPipelineDepthStencilStateCreateInfo depth_stencil_state(bool depth_test, bool depth_write, VkCompareOp compare_op);

	VkPipelineShaderStageCreateInfo shader_stage(VkShaderStageFlagBits flag, VkShaderModule shader_module, const VkSpecializationInfo* specialization = nullptr);
	VkPipelineShaderStageCreateInfo shader_stage(const lvk_shader_module* shader_module, const VkSpecializationInfo* specialization = nullptr);

	VkPipelineVertexInputStateCreateInfo vertex_input_state(const VkVertexInputBindingDescription* binding_description, uint32_t binding_description_count, const VkVertexInputAttributeDescription* attribute_description, uint32_t attribute_description_count);
	template <std::size_t B, std::size_t A>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineVertexInputStateCreateInfo vertex_input_state(const VkVertexInputBindingDescription (&bindings)[B], const VkVertexInputAttributeDescription (&attributes)[A]) noexcept {
		return {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = static_cast<uint32_t>(B),
			.pVertexBindingDescriptions = bindings,
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(A),
			.pVertexAttributeDescriptions = attributes
		};
	}
	
	VkPipelineInputAssemblyStateCreateInfo input_assembly_state(const VkPrimitiveTopology topology, bool primitive_restart_enable);
	
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(const VkPolygonMode polygon_mode, const VkCullModeFlags cull_mode, const VkFrontFace front_face, float line_width = 1.0, const bool depth_clamp_enable = false, const bool discard_rasterizer = false, const bool depth_bias_enable = false, const float depth_bias_constant_factor = 0, float depth_bias_clamp = 0, float depth_bias_slope_factor = 0);
	VkPipelineRasterizationStateCreateInfo rasterization_state(const VkPolygonMode polygon_mode);
	
	VkPipelineMultisampleStateCreateInfo multisample_state_create_info(VkSampleCountFlagBits rasterization_sample, bool sample_shading, float min_sample_shading = 1.0, const VkSampleMask* sample_mask = VK_NULL_HANDLE, bool alpha_to_coverage = false, bool alpha_to_one = false);
	VkPipelineMultisampleStateCreateInfo multisample_state();

	VkPipelineColorBlendStateCreateInfo color_blend_state(const VkPipelineColorBlendAttachmentState* attachments, const uint32_t attachment_count, const bool logic_op_enable = false, const VkLogicOp logic_op = VK_LOGIC_OP_COPY);

	VkImageViewCreateInfo image_view(VkImage image, VkFormat format, VkImageViewType view_type, VkImageSubresourceRange subresource_range, VkComponentMapping components);
	VkImageViewCreateInfo image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flag, VkImageViewType view_type);
}