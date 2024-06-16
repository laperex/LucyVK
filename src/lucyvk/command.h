#pragma once

#include "lucyvk/synchronization.h"
#include "lucyvk/types.h"
#include <vulkan/vulkan_core.h>


// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


struct lvk_command_pool {
	VkCommandPool _command_pool;
};


// |--------------------------------------------------
// ----------------> COMMAND BUFFER
// |--------------------------------------------------


struct lvk_command_buffer {
	VkCommandBuffer _command_buffer;

	// const lvk_command_pool* command_pool;
	// const lvk_device* device;

	void reset(VkCommandBufferResetFlags flags = 0);

	void begin(const VkCommandBufferBeginInfo* beginInfo);
	void begin(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info = VK_NULL_HANDLE);

	// void transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);
	// void transition_image(const lvk_image* image, VkImageLayout current_layout, VkImageLayout new_layout);
	
	void blit_image_to_image(VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size);
	
	void bind_pipeline(const lvk_pipeline* pipeline);

	void bind_descriptor_set(const lvk_pipeline* pipeline, const lvk_descriptor_set* descriptor_set, const uint32_t descript_set_count);

	template <std::size_t _ds_N> [[__gnu__::__always_inline__]]
	constexpr void bind_descriptor_set(const lvk_pipeline* pipeline, const lvk_descriptor_set (&descriptor_set)[_ds_N]) noexcept {
		return bind_descriptor_set(pipeline, descriptor_set, _ds_N);
	}
	
	void bind_vertex_buffers(const VkBuffer* vertex_buffers, const VkDeviceSize* offset_array, const uint32_t vertex_buffers_count, const uint32_t first_binding = 0);
	template <std::size_t _b_m> [[__gnu__::__always_inline__]]
    constexpr void bind_vertex_buffers(const VkBuffer (&vertex_buffers)[_b_m], const VkDeviceSize (&offset_array)[_b_m], const uint32_t first_binding = 0) noexcept {
		bind_vertex_buffers(vertex_buffers, offset_array, _b_m, first_binding);
	}
	
	template <std::size_t _cv_N> [[__gnu__::__always_inline__]]
	constexpr void begin_render_pass(const lvk_framebuffer* framebuffer, const VkSubpassContents subpass_contents, const VkClearValue (&clear_values)[_cv_N]) noexcept {
		begin_render_pass(framebuffer, subpass_contents, clear_values, _cv_N);
	}
	void begin_render_pass(const lvk_framebuffer* framebuffer, const VkSubpassContents subpass_contents, const VkClearValue* clear_values, const uint32_t clear_value_count);
	void begin_render_pass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents subpass_contents);
	
	void end();
	void end_render_pass();

	void copy_image_to_image(VkImage source, VkImage destination, VkImageLayout source_layout, VkImageLayout destination_layout, VkExtent2D src_size, VkExtent2D dst_size);
	
	void dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z);

	void transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);
};


// |--------------------------------------------------
// ----------------> IMMEDIATE COMMAND BUFFER
// |--------------------------------------------------


struct lvk_immediate_command_buffer {
	VkCommandBuffer _command_buffer;
	VkFence _fence;

	void transition_image(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) const;
};