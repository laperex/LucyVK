#pragma once

#include "lucyvk/vk_types.h"
#include <vulkan/vulkan_core.h>

namespace lvk::info {
	VkShaderModuleCreateInfo shader_module(const char* filename);
	
	VkPipelineDepthStencilStateCreateInfo depth_stencil_state(bool depth_test, bool depth_write, VkCompareOp compare_op);

	VkPipelineShaderStageCreateInfo shader_stage(VkShaderStageFlagBits flag, VkShaderModule shader_module, const VkSpecializationInfo* specialization = nullptr);
	VkPipelineShaderStageCreateInfo shader_stage(const lvk_shader_module* shader_module, const VkSpecializationInfo* specialization = nullptr);

	VkPipelineVertexInputStateCreateInfo vertex_input_state(const VkVertexInputBindingDescription* binding_description = {}, uint32_t binding_description_count = 0, const VkVertexInputAttributeDescription* attribute_description = {}, uint32_t attribute_description_count = 0);
	template <std::size_t _b_N, std::size_t _a_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineVertexInputStateCreateInfo vertex_input_state(const VkVertexInputBindingDescription (&bindings)[_b_N], const VkVertexInputAttributeDescription (&attributes)[_a_N]) noexcept {
		return vertex_input_state(bindings, _b_N, attributes, _a_N);
	}
	template <std::size_t _b_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineVertexInputStateCreateInfo vertex_input_state(const VkVertexInputBindingDescription (&bindings)[_b_N]) noexcept {
		return vertex_input_state(bindings, _b_N, nullptr, 0);
	}
	template <std::size_t _a_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineVertexInputStateCreateInfo vertex_input_state(const VkVertexInputAttributeDescription (&attributes)[_a_N]) noexcept {
		return vertex_input_state(nullptr, 0, attributes, _a_N);
	}
	
	VkPipelineInputAssemblyStateCreateInfo input_assembly_state(const VkPrimitiveTopology topology, bool primitive_restart_enable);
	
	VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(const VkPolygonMode polygon_mode, const VkCullModeFlags cull_mode, const VkFrontFace front_face, float line_width = 1.0, const bool depth_clamp_enable = false, const bool discard_rasterizer = false, const bool depth_bias_enable = false, const float depth_bias_constant_factor = 0, float depth_bias_clamp = 0, float depth_bias_slope_factor = 0);
	VkPipelineRasterizationStateCreateInfo rasterization_state(const VkPolygonMode polygon_mode);
	
	VkPipelineMultisampleStateCreateInfo multisample_state_create_info(VkSampleCountFlagBits rasterization_sample, bool sample_shading, float min_sample_shading = 1.0, const VkSampleMask* sample_mask = VK_NULL_HANDLE, bool alpha_to_coverage = false, bool alpha_to_one = false);
	VkPipelineMultisampleStateCreateInfo multisample_state();

	VkPipelineColorBlendStateCreateInfo color_blend_state(const VkPipelineColorBlendAttachmentState* attachments = nullptr, const uint32_t attachment_count = 0, const bool logic_op_enable = false, const VkLogicOp logic_op = VK_LOGIC_OP_COPY);
	template <std::size_t _pcbas_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineColorBlendStateCreateInfo color_blend_state(const VkPipelineColorBlendAttachmentState (&attachments)[_pcbas_N], const bool logic_op_enable = false, const VkLogicOp logic_op = VK_LOGIC_OP_COPY) noexcept {
		return color_blend_state(attachments, _pcbas_N, logic_op_enable, logic_op);
	}
	
	VkPipelineViewportStateCreateInfo viewport_state(const VkViewport* viewports, const uint32_t viewports_count, const VkRect2D* scissors, const uint32_t scissors_count) noexcept;
	template <std::size_t _v_N, std::size_t _s_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineViewportStateCreateInfo viewport_state(const VkViewport (&viewports)[_v_N], const VkRect2D (&scissors)[_s_N]) noexcept {
		return viewport_state(viewports, _v_N, scissors, _s_N);
	}
	template <std::size_t _s_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineViewportStateCreateInfo viewport_state(const VkRect2D (&scissors)[_s_N]) noexcept {
		return viewport_state(nullptr, 0, scissors, _s_N);
	}
	template <std::size_t _v_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineViewportStateCreateInfo viewport_state(const VkViewport (&viewports)[_v_N]) noexcept {
		return viewport_state(viewports, _v_N, nullptr, 0);
	}

	VkImageViewCreateInfo image_view(VkImage image, VkFormat format, VkImageViewType view_type, VkImageSubresourceRange subresource_range, VkComponentMapping components);
	VkImageViewCreateInfo image_view(VkImage image, VkFormat format, VkImageAspectFlags aspect_flag, VkImageViewType view_type);
	
	VkSemaphoreSubmitInfo semaphore_submit(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
	
	VkCommandBufferSubmitInfo command_buffer_submit(const lvk_command_buffer* command_buffer);

	VkSubmitInfo2 submit2(const VkCommandBufferSubmitInfo* command_buffer_infos, const uint32_t command_buffer_infos_count, const VkSemaphoreSubmitInfo* signal_semaphore_infos, const uint32_t signal_semaphore_infos_count, const VkSemaphoreSubmitInfo* wait_semaphore_infos, const uint32_t wait_semaphore_infos_count);
	template <std::size_t _cbsi_N, std::size_t _sssi_N, std::size_t _wssi_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkSubmitInfo2 submit2(const VkCommandBufferSubmitInfo (&command_buffer_infos)[_cbsi_N], const VkSemaphoreSubmitInfo (&signal_semaphore_infos)[_sssi_N], const VkSemaphoreSubmitInfo (&wait_semaphore_infos)[_wssi_N]) {
		return submit2(command_buffer_infos, _cbsi_N, signal_semaphore_infos, _sssi_N, wait_semaphore_infos, _wssi_N);
	}
}