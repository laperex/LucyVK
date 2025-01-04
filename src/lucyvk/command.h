#pragma once

#include <cstddef>
#include <functional>
#include "lucyvk/handles.h"




// |--------------------------------------------------
// ----------------> COMMAND BUFFER
// |--------------------------------------------------


struct lvk_command_buffer {
	LVK_HANDLE_DEF(VkCommandBuffer, _command_buffer)

	void reset(VkCommandBufferResetFlags flags = 0) const;

	void begin(const VkCommandBufferBeginInfo* beginInfo) const;
	void begin(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info = VK_NULL_HANDLE) const;

	void blit_image_to_image2(VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size) const;

	void set_viewport(const VkViewport viewport) const;
	void set_viewport(const uint32_t first, const VkViewport* viewports, const uint32_t count) const;
	template <std::size_t _v_N>  [[__gnu__::__always_inline__]]
	constexpr void set_viewport(const uint32_t first, const VkViewport (&viewports)[_v_N]) const noexcept {
		set_viewport(first, viewports, _v_N);
	}
	template <std::size_t _v_N>  [[__gnu__::__always_inline__]]
	constexpr void set_viewport(const VkViewport (&viewports)[_v_N]) const noexcept {
		set_viewport(0, viewports, _v_N);
	}

	void set_scissor(const VkRect2D scissors) const;
	void set_scissor(const uint32_t first, const VkRect2D* scissors, const uint32_t count) const;
	template <std::size_t _v_N>  [[__gnu__::__always_inline__]]
	constexpr void set_scissor(const uint32_t first, const VkRect2D (&scissors)[_v_N]) const noexcept {
		set_scissor(first, scissors, _v_N);
	}
	template <std::size_t _v_N>  [[__gnu__::__always_inline__]]
	constexpr void set_scissor(const VkRect2D (&scissors)[_v_N]) const noexcept {
		set_scissor(0, scissors, _v_N);
	}

	void bind_pipeline(const VkPipelineBindPoint pipeline_bind_point, const VkPipeline pipeline) const;

	void bind_descriptor_set(const VkPipelineBindPoint pipeline_bind_point, const VkPipelineLayout pipeline_layout, const lvk_descriptor_set* descriptor_set, const uint32_t descriptor_set_count, uint32_t first_set = 0) const;

	template <std::size_t _ds_N> [[__gnu__::__always_inline__]]
	constexpr void bind_descriptor_set(const VkPipelineBindPoint pipeline_bind_point, const VkPipelineLayout pipeline_layout, const lvk_descriptor_set (&descriptor_set)[_ds_N], uint32_t first_set = 0) const noexcept {
		return bind_descriptor_set(pipeline_bind_point, pipeline_layout, descriptor_set, _ds_N, first_set);
	}
	
	void bind_index_buffer(const VkBuffer index_buffer, const VkIndexType index_type, const VkDeviceSize offset = 0) const;
	
	void bind_vertex_buffers(const VkBuffer* vertex_buffers, const VkDeviceSize* offset_array, const uint32_t vertex_buffers_count, const uint32_t first_binding = 0) const;
	template <std::size_t _b_m> [[__gnu__::__always_inline__]]
    constexpr void bind_vertex_buffers(const VkBuffer (&vertex_buffers)[_b_m], const VkDeviceSize (&offset_array)[_b_m] = {}, const uint32_t first_binding = 0) const noexcept {
		bind_vertex_buffers(vertex_buffers, offset_array, _b_m, first_binding);
	}
	
	template <std::size_t _cv_N> [[__gnu__::__always_inline__]]
	constexpr void begin_render_pass(const VkRenderPass render_pass, const VkFramebuffer framebuffer, const VkExtent2D extent, const VkSubpassContents subpass_contents, const VkClearValue (&clear_values)[_cv_N]) const noexcept {
		begin_render_pass(render_pass, framebuffer, extent, subpass_contents, clear_values, _cv_N);
	}
	void begin_render_pass(const VkRenderPass render_pass, const VkFramebuffer framebuffer, const VkExtent2D extent, const VkSubpassContents subpass_contents, const VkClearValue* clear_values, const uint32_t clear_value_count) const;
	void begin_render_pass(const VkRenderPassBeginInfo beginInfo, const VkSubpassContents subpass_contents) const;
	
	void draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) const;
	void draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const;
	
	void end() const;
	void end_render_pass() const;

	void copy_image_to_image2(VkImage source, VkImage destination, VkImageLayout source_layout, VkImageLayout destination_layout, VkExtent2D src_size, VkExtent2D dst_size) const;
	void copy_buffer_to_image(VkBuffer source, VkImage destination, VkExtent3D region) const;
	
	void copy_buffer_to_buffer(const VkBuffer src_buffer, const VkBuffer dst_buffer, const VkBufferCopy* buffer_copy_array, const uint32_t buffer_copy_array_size) const;
	template <std::size_t _bc_N> [[__gnu__::__always_inline__]]
	constexpr void copy_buffer_to_buffer(const VkBuffer src_buffer, const VkBuffer dst_buffer, const VkBufferCopy (&buffer_copy_array)[_bc_N]) const noexcept {
		copy_buffer_to_buffer(src_buffer, dst_buffer, buffer_copy_array, _bc_N);
	}

	void dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) const;

	void transition_image2(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const;
	// VkSubmitInfo immediate_transition_image2(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const;
	
	void transition_image(VkImageMemoryBarrier image_memory_barrier) const;
	void pipeline_barrier(VkPipelineStageFlags src_pipeline_stage, VkPipelineStageFlags dst_pipeline_stage, VkImageMemoryBarrier image_memory_barrier) const;
	// VkSubmitInfo immediate_transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const;
};
