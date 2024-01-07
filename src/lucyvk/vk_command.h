#pragma once

#include "lucyvk/vk_types.h"
#include <vulkan/vulkan_core.h>



// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


struct lvk_command_pool {
	VkCommandPool _command_pool;

	// ~lvk_command_pool();

	const lvk_instance* instance;
	const lvk_physical_device* physical_device;
	const lvk_device* device;
	
	lvk::deletion_queue* deletion_queue;

	lvk_command_buffer init_command_buffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
};


// |--------------------------------------------------
// ----------------> COMMAND BUFFER
// |--------------------------------------------------


struct lvk_command_buffer {
	VkCommandBuffer _command_buffer;

	const lvk_command_pool* command_pool;
	const lvk_device* device;

	void reset(VkCommandBufferResetFlags flags = 0);

	void begin(const VkCommandBufferBeginInfo* beginInfo);
	void begin(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info = VK_NULL_HANDLE);

	void transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);
	void transition_image(const lvk_image* image, VkImageLayout current_layout, VkImageLayout new_layout);
	
	void blit_image_to_image(VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size);
	
	void bind_pipeline(const lvk_pipeline* pipeline);
	
	void bind_vertex_buffer(const lvk_buffer* vertex_buffer, const VkDeviceSize offset);

	void bind_vertex_buffers(const VkBuffer* vertex_buffers, const VkDeviceSize* offset_array, const uint32_t vertex_buffers_count, const uint32_t first_binding = 0);
	template <std::size_t _Nm> [[__gnu__::__always_inline__]]
    constexpr void bind_vertex_buffers(const VkBuffer (&vertex_buffers)[_Nm], const VkDeviceSize (&offset_array)[_Nm], const uint32_t first_binding = 0) noexcept {
		bind_vertex_buffers(vertex_buffers, offset_array, _Nm, first_binding);
	}
	
	template <std::size_t _cv_N> [[__gnu__::__always_inline__]]
	constexpr void begin_render_pass(const lvk_framebuffer* framebuffer, const VkSubpassContents subpass_contents, const VkClearValue (&clear_values)[_cv_N]) noexcept {
		begin_render_pass(framebuffer, subpass_contents, clear_values, _cv_N);
	}
	void begin_render_pass(const lvk_framebuffer* framebuffer, const VkSubpassContents subpass_contents, const VkClearValue* clear_values, const uint32_t clear_value_count);
	void begin_render_pass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents subpass_contents);
	
	void bind(lvk_pipeline);

	void end();
	void end_render_pass();

	void copy_image_to_image(VkImage source, VkImage destination, VkImageLayout source_layout, VkImageLayout destination_layout, VkExtent2D src_size, VkExtent2D dst_size);
};