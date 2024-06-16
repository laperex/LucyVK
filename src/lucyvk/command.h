#pragma once

#include <cstddef>
#include <functional>
#include "lucyvk/handles.h"




// |--------------------------------------------------
// ----------------> COMMAND BUFFER
// |--------------------------------------------------


struct lvk_command_buffer {
	VkCommandBuffer _command_buffer;

	// const lvk_command_pool* command_pool;
	// const lvk_device* device;

	void reset(VkCommandBufferResetFlags flags = 0) const;

	void begin(const VkCommandBufferBeginInfo* beginInfo) const;
	void begin(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info = VK_NULL_HANDLE) const;

	// void transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const;
	// void transition_image(const lvk_image* image, VkImageLayout current_layout, VkImageLayout new_layout) const;
	
	void blit_image_to_image(VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size) const;
	
	void bind_pipeline(const lvk_pipeline* pipeline) const;

	void bind_descriptor_set(const lvk_pipeline_layout& pipeline_layout, const lvk_pipeline& pipeline, const lvk_descriptor_set* descriptor_set, const uint32_t descriptor_set_count) const;

	template <std::size_t _ds_N> [[__gnu__::__always_inline__]]
	constexpr void bind_descriptor_set(const lvk_pipeline_layout& pipeline_layout, const lvk_pipeline& pipeline, const lvk_descriptor_set (&descriptor_set)[_ds_N]) const noexcept {
		return bind_descriptor_set(pipeline_layout, pipeline, descriptor_set, _ds_N);
	}
	
	void bind_vertex_buffers(const VkBuffer* vertex_buffers, const VkDeviceSize* offset_array, const uint32_t vertex_buffers_count, const uint32_t first_binding = 0) const;
	template <std::size_t _b_m> [[__gnu__::__always_inline__]]
    constexpr void bind_vertex_buffers(const VkBuffer (&vertex_buffers)[_b_m], const VkDeviceSize (&offset_array)[_b_m], const uint32_t first_binding = 0) const noexcept {
		bind_vertex_buffers(vertex_buffers, offset_array, _b_m, first_binding);
	}
	
	template <std::size_t _cv_N> [[__gnu__::__always_inline__]]
	constexpr void begin_render_pass(const lvk_framebuffer* framebuffer, const VkSubpassContents subpass_contents, const VkClearValue (&clear_values)[_cv_N]) const noexcept {
		begin_render_pass(framebuffer, subpass_contents, clear_values, _cv_N);
	}
	void begin_render_pass(const lvk_render_pass render_pass, const lvk_framebuffer& framebuffer, const VkSubpassContents subpass_contents, const VkClearValue* clear_values, const uint32_t clear_value_count) const;
	void begin_render_pass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents subpass_contents) const;
	
	void end() const;
	void end_render_pass() const;

	void copy_image_to_image(VkImage source, VkImage destination, VkImageLayout source_layout, VkImageLayout destination_layout, VkExtent2D src_size, VkExtent2D dst_size) const;
	
	void dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) const;

	void transition_image2(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const;
	VkSubmitInfo immediate_transition_image2(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const;
	
	void transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const;
	VkSubmitInfo immediate_transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const;

	VkSubmitInfo immediate(std::function<void()> function) const;
};
