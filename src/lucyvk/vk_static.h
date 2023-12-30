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
// ----------------> INSTANCE
// |--------------------------------------------------


lvk_instance lvk_init_instance(const lvk::config::instance* config, SDL_Window* sdl_window);

struct lvk_instance {
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkSurfaceKHR _surface;
	VkInstance _instance;

	std::vector<const char*> layers = {};
	
	~lvk_instance();

	bool is_debug_enable();

	lvk_physical_device init_physical_device(lvk::SelectPhysicalDeviceFunction function = nullptr);
};


// |--------------------------------------------------
// ----------------> PHYSICAL DEVICE
// |--------------------------------------------------


struct lvk_physical_device {
	VkPhysicalDevice _physical_device;

	VkPhysicalDeviceFeatures _features;
	VkPhysicalDeviceProperties _properties;

	lvk::queue_family_indices _queue_family_indices;
	lvk::swapchain_support_details _swapchain_support_details;
	
	const lvk_instance* instance;

	lvk_device init_device(std::vector<const char*> layers = {}, std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME });

	const VkFormat find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	const uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags property_flags) const;
};


// |--------------------------------------------------
// ----------------> DEVICE
// |--------------------------------------------------


struct lvk_device {
	VkDevice _device;
	
	VkQueue _present_queue;
	VkQueue _compute_queue;
	VkQueue _transfer_queue;
	VkQueue _graphics_queue;

	std::vector<const char*> extensions = {};
	std::vector<const char*> layers = {};
	
	~lvk_device();

	const lvk_physical_device* physical_device;
	const lvk_instance* instance;
	
	lvk::deletion_queue deletion_queue;

	lvk_swapchain init_swapchain(uint32_t width, uint32_t height);
	
	lvk_command_pool init_command_pool();
	lvk_command_pool init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags);
	
	lvk_queue init_queue();
	
	lvk_render_pass init_render_pass();
	lvk_render_pass init_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform = false);

	lvk_semaphore init_semaphore();
	lvk_fence init_fence(VkFenceCreateFlags flags = 0);

	lvk_shader_module init_shader_module(VkShaderStageFlagBits stage, const char* filename);
	
	lvk_pipeline_layout init_pipeline_layout(const VkPushConstantRange* push_constant_ranges, uint32_t push_constant_range_count, const VkDescriptorSetLayout* descriptor_set_layouts, uint32_t descriptor_set_layout_count);
	
	lvk_allocator init_allocator();
	
	lvk_descriptor_set_layout init_descriptor_set_layout(const VkDescriptorSetLayoutBinding* bindings, const uint32_t binding_count);
	lvk_descriptor_pool init_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize* descriptor_pool_sizes, const uint32_t descriptor_pool_sizes_count);

	void wait_idle() const;

	VkResult submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const lvk_fence* fence, uint64_t timeout = LVK_TIMEOUT) const;
	VkResult present(const VkPresentInfoKHR* present_info) const;
};


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


struct lvk_swapchain {
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	VkExtent2D _extent = { 0, 0 };
	VkSurfaceFormatKHR _surface_format;
	VkPresentModeKHR _present_mode;

	uint32_t _image_count;
	VkImage* _images;
	VkImageView* _image_views;

	bool recreate(const uint32_t width, const uint32_t height);
	VkResult acquire_next_image(uint32_t* index, VkSemaphore semaphore, VkFence fence, const uint64_t timeout = LVK_TIMEOUT);
	
	~lvk_swapchain();
	
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

	void begin_render_pass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents subpass_contents);
	void begin_render_pass(const lvk_framebuffer* framebuffer, const VkClearValue* clear_values, const uint32_t clear_value_count, const VkSubpassContents subpass_contents);
	
	void bind(lvk_pipeline);

	void end();
	void end_render_pass();
};


// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


struct lvk_render_pass {
	VkRenderPass _render_pass;

	~lvk_render_pass();

	const lvk_device* device;
	const lvk_physical_device* physical_device;
	const lvk_instance* instance;

	lvk_framebuffer init_framebuffer(const VkExtent2D extent, const VkImageView* image_views, const uint32_t image_views_count);

	lvk::deletion_queue deletion_queue;
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