#pragma once

#include "vk_mem_alloc.h"

#include "vk_types.h"
#include "vk_config.h"

#include <SDL_video.h>
#include <functional>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
// #include <vulkan/vulkan_>

// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


struct lvk_swapchain {
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	VkExtent2D _extent = { 0, 0 };
	VkSurfaceFormatKHR _surface_format;
	VkPresentModeKHR _present_mode;
	VkImageUsageFlags _image_usage;

	uint32_t _image_count;
	VkImage* _images;
	VkImageView* _image_views;

	bool recreate(const uint32_t width, const uint32_t height);
	VkResult acquire_next_image(uint32_t* index, VkSemaphore semaphore, VkFence fence, const uint64_t timeout = LVK_TIMEOUT);
	
	const lvk_device* device;
	const lvk_physical_device* physical_device;
	const lvk_instance* instance;
};


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
	
	void bind_pipeline(const lvk_pipeline* pipeline);
	
	void bind_vertex_buffer(const lvk_buffer* vertex_buffer, const VkDeviceSize offset);

	void bind_vertex_buffers(const VkBuffer* vertex_buffers, const VkDeviceSize* offset_array, const uint32_t vertex_buffers_count);
	template <std::size_t _Nm> [[__gnu__::__always_inline__]]
    constexpr void bind_vertex_buffers(const VkBuffer (&vertex_buffers)[_Nm], const VkDeviceSize (&offset_array)[_Nm]) noexcept {
		bind_vertex_buffers(vertex_buffers, offset_array, _Nm);
	}

	void transition_image(const lvk_image* image, VkImageLayout current_layout, VkImageLayout new_layout);

	template<std::size_t _cv_N> [[__gnu__::__always_inline__]]
	constexpr void begin_render_pass(const lvk_framebuffer* framebuffer, const VkSubpassContents subpass_contents, const VkClearValue (&clear_values)[_cv_N]) noexcept {
		begin_render_pass(framebuffer, subpass_contents, clear_values, _cv_N);
	}
	void begin_render_pass(const lvk_framebuffer* framebuffer, const VkSubpassContents subpass_contents, const VkClearValue* clear_values, const uint32_t clear_value_count);
	void begin_render_pass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents subpass_contents);
	
	void bind(lvk_pipeline);

	void end();
	void end_render_pass();
};


// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


struct lvk_render_pass {
	VkRenderPass _render_pass;

	const lvk_device* device;
	const lvk_physical_device* physical_device;
	const lvk_instance* instance;
	
	lvk::deletion_queue* deletion_queue;

	lvk_framebuffer init_framebuffer(const VkExtent2D extent, const VkImageView* image_views, const uint32_t image_views_count);
};


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


struct lvk_framebuffer {
	VkFramebuffer _framebuffer;
	VkExtent2D _extent;

	const lvk_render_pass* render_pass;
	const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


struct lvk_semaphore {
	VkSemaphore _semaphore;

	const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


struct lvk_fence {
	VkFence _fence;

	const lvk_device* device;

	VkResult wait(uint64_t timeout = LVK_TIMEOUT) const;
	VkResult reset() const;
};


// ! |--------------------------------------------------
// ! ----------------> SHADER ( OBSELETE SHOULD BE REMOVED AFTER TESTING )
// ! |--------------------------------------------------

struct lvk_shader_module {
	VkShaderModule _shader_module;

	const VkShaderStageFlagBits _stage;

	~lvk_shader_module();

	const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


struct lvk_pipeline_layout {
	VkPipelineLayout _pipeline_layout;
	
	// const lvk_render_pass* render_pass;
	const lvk_device* device;
	
	lvk::deletion_queue* deletion_queue;

	lvk_pipeline init_graphics_pipeline(const lvk_render_pass* render_pass, const lvk::config::graphics_pipeline* config);
};


// |--------------------------------------------------
// ----------------> PIPELINE
// |--------------------------------------------------


struct lvk_pipeline {
	VkPipeline _pipeline;
	
	const lvk_pipeline_layout* pipeline_layout;
	const lvk_render_pass* render_pass;
	const lvk_device* device;
	
	VkPipelineBindPoint type;
	
	lvk::deletion_queue* deletion_queue;
	
	~lvk_pipeline();
};


// |--------------------------------------------------
// ----------------> ALLOCATOR
// |--------------------------------------------------


struct lvk_allocator {
	VmaAllocator _allocator;
	
	const lvk_device* device;

	lvk::deletion_queue* deletion_queue;
	
	lvk_buffer init_buffer(VkBufferUsageFlagBits buffer_usage, VmaMemoryUsage memory_usage, const void* data, const std::size_t size);

	lvk_buffer init_vertex_buffer(const void* data, const std::size_t size);
	lvk_buffer init_uniform_buffer(const void* data, const std::size_t size);

	lvk_image init_image(VkFormat format, VkImageUsageFlags usage, VkImageType image_type, VkExtent3D extent);
};


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


struct lvk_buffer {
	VkBuffer _buffer;
	VmaAllocation _allocation;
	
	std::size_t _allocated_size;
	VkBufferUsageFlagBits _usage;

	const lvk_allocator* allocator;

	void upload(const void* data, const std::size_t size) const;

	template <typename T> inline void upload(const T& data) const { upload(&data, sizeof(T)); }
};


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------


struct lvk_image {
	VkImage _image;
	VmaAllocation _allocation;
	
	VkFormat _format;
	VkImageType _image_type;
	VkExtent3D _extent;
	VkImageUsageFlags _usage;
	
	const lvk_allocator* allocator;
	const lvk_device* device;

	lvk::deletion_queue* deletion_queue;
	
	lvk_image_view init_image_view(VkImageAspectFlags aspect_flag, VkImageViewType image_view_type);
};


// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


struct lvk_image_view {
	VkImageView _image_view;

	const lvk_image* image;
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET LAYOUT
// |--------------------------------------------------


struct lvk_descriptor_set_layout {
	VkDescriptorSetLayout _descriptor_set_layout;
	
	const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR POOL
// |--------------------------------------------------


struct lvk_descriptor_pool {
	VkDescriptorPool _descriptor_pool;
	
	const lvk_device* device;
	
	lvk_descriptor_set init_descriptor_set(const lvk_descriptor_set_layout* descriptor_set_layout);
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


struct lvk_descriptor_set {
	VkDescriptorSet _descriptor_set;
	
	const lvk_descriptor_pool* descriptor_pool;
	const lvk_device* device;
	
	void update(const lvk_buffer* buffer, const std::size_t offset = 0) const;
};
