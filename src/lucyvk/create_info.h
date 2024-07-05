#pragma once

#include <vector>

// #include "lucyvk/types.h"
#include "lucyvk/handles.h"
#include <vulkan/vulkan_core.h>

namespace lvk::info {
	VkShaderModuleCreateInfo shader_module(const char* filename);
	
	VkPipelineDepthStencilStateCreateInfo depth_stencil_state(bool depth_test, bool depth_write, VkCompareOp compare_op);

	VkPipelineShaderStageCreateInfo shader_stage(VkShaderStageFlagBits flag, VkShaderModule shader_module, const char* main = "main", const VkSpecializationInfo* specialization = nullptr);

	VkPipelineVertexInputStateCreateInfo vertex_input_state(const VkVertexInputBindingDescription* binding_description = {}, uint32_t binding_description_count = 0, const VkVertexInputAttributeDescription* attribute_description = {}, uint32_t attribute_description_count = 0);
	VkPipelineVertexInputStateCreateInfo vertex_input_state(const std::vector<VkVertexInputBindingDescription>& binding_description = {}, const std::vector<VkVertexInputAttributeDescription>& attribute_description = {});
	template <std::size_t _b_N, std::size_t _a_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineVertexInputStateCreateInfo vertex_input_state(const VkVertexInputBindingDescription (&bindings)[_b_N], const VkVertexInputAttributeDescription (&attributes)[_a_N]) noexcept {
		return vertex_input_state(bindings, _b_N, attributes, _a_N);
	}
	
	VkPipelineInputAssemblyStateCreateInfo input_assembly_state(const VkPrimitiveTopology topology, bool primitive_restart_enable);
	
	VkPipelineRasterizationStateCreateInfo rasterization_state(const VkPolygonMode polygon_mode, const VkCullModeFlags cull_mode, const VkFrontFace front_face, float line_width = 1.0, const bool depth_clamp_enable = false, const bool discard_rasterizer = false, const bool depth_bias_enable = false, const float depth_bias_constant_factor = 0, float depth_bias_clamp = 0, float depth_bias_slope_factor = 0);
	VkPipelineRasterizationStateCreateInfo rasterization_state(const VkPolygonMode polygon_mode);
	
	VkPipelineMultisampleStateCreateInfo multisample_state_create_info(VkSampleCountFlagBits rasterization_sample, bool sample_shading, float min_sample_shading = 1.0, const VkSampleMask* sample_mask = VK_NULL_HANDLE, bool alpha_to_coverage = false, bool alpha_to_one = false);
	VkPipelineMultisampleStateCreateInfo multisample_state();

	VkPipelineColorBlendAttachmentState color_blend_attachment();

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

	VkPipelineRenderingCreateInfo rendering(const VkFormat depth_attachment_format = VK_FORMAT_UNDEFINED, const VkFormat stencil_attachment_format = VK_FORMAT_UNDEFINED, const VkFormat* color_attachment_formats = nullptr, const uint32_t color_attachment_formats_count = 0);
	template <std::size_t _f_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkPipelineRenderingCreateInfo rendering(const VkFormat (&color_attachment_formats)[_f_N], const VkFormat depth_attachment_format = VK_FORMAT_UNDEFINED, const VkFormat stencil_attachment_format = VK_FORMAT_UNDEFINED) noexcept {
		return rendering(depth_attachment_format, stencil_attachment_format, color_attachment_formats, _f_N);
	}

	VkImageViewCreateInfo image_view(const VkImage image, VkFormat format, VkImageViewType view_type, VkImageSubresourceRange subresource_range, VkComponentMapping components);
	VkImageViewCreateInfo image_view(const VkImage image, VkFormat format, VkImageAspectFlags aspect_flag, VkImageViewType view_type);
	
	VkSemaphoreSubmitInfo semaphore_submit(VkPipelineStageFlags2 stage_mask, const VkSemaphore semaphore);
	
	VkCommandBufferSubmitInfo command_buffer_submit(const lvk_command_buffer* command_buffer);

	VkSubmitInfo submit(const VkCommandBuffer* command_buffers, const uint32_t command_buffer_count, const VkSemaphore* signal_semaphores, const uint32_t signal_semaphores_count, const VkSemaphore* wait_semaphores, const uint32_t wait_semaphore_count);
	VkSubmitInfo2 submit2(const VkCommandBufferSubmitInfo* command_buffer_infos, const uint32_t command_buffer_infos_count, const VkSemaphoreSubmitInfo* signal_semaphore_infos, const uint32_t signal_semaphore_infos_count, const VkSemaphoreSubmitInfo* wait_semaphore_infos, const uint32_t wait_semaphore_infos_count);
	template <std::size_t _cbsi_N, std::size_t _sssi_N, std::size_t _wssi_N>  [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkSubmitInfo2 submit2(const VkCommandBufferSubmitInfo (&command_buffer_infos)[_cbsi_N], const VkSemaphoreSubmitInfo (&signal_semaphore_infos)[_sssi_N], const VkSemaphoreSubmitInfo (&wait_semaphore_infos)[_wssi_N]) noexcept {
		return submit2(command_buffer_infos, _cbsi_N, signal_semaphore_infos, _sssi_N, wait_semaphore_infos, _wssi_N);
	}

	VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount);
	VkImageSubresourceRange image_subresource_range(VkImageAspectFlags aspect_mask);
	
	VkImageMemoryBarrier image_memory_barrier(const VkImage image, VkAccessFlags src_access, VkAccessFlags dst_access, VkImageLayout old_layout, VkImageLayout new_layout, uint32_t src_queue_index, uint32_t dst_queue_index, VkImageSubresourceRange subresource_range);
	VkImageMemoryBarrier image_memory_barrier(const VkImage image, VkAccessFlags src_access, VkAccessFlags dst_access, VkImageLayout old_layout, VkImageLayout new_layout, VkImageSubresourceRange subresource_range);

	VkDescriptorSetLayoutBinding descriptor_set_layout_binding(uint32_t binding, VkShaderStageFlags shader_stage_flags, VkDescriptorType descriptor_type, uint32_t descriptor_count);
	
	VkAttachmentReference attachment_reference(uint32_t attachment, VkImageLayout layout);
	
	VkVertexInputBindingDescription vertex_input_description(uint32_t binding, uint32_t stride, VkVertexInputRate input_rate);
	template <typename T>
	VkVertexInputBindingDescription vertex_input_description(uint32_t binding, VkVertexInputRate input_rate) {
		return vertex_input_description(binding, sizeof(T), input_rate);
	}
	
	VkVertexInputAttributeDescription vertex_input_attribute_description(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);
	
	VkSubpassDescription subpass_description(VkSubpassDescriptionFlags flags, VkPipelineBindPoint pipelineBindPoint, uint32_t inputAttachmentCount, const VkAttachmentReference* pInputAttachments, uint32_t colorAttachmentCount, const VkAttachmentReference* pColorAttachments, const VkAttachmentReference* pResolveAttachments, const VkAttachmentReference* pDepthStencilAttachment, uint32_t preserveAttachmentCount, const uint32_t* pPreserveAttachments);
}