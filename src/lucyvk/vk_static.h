#pragma once

#include "vk_mem_alloc.h"

#include "vk_types.h"

#include <SDL_video.h>
#include <functional>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>
// #include <vulkan/vulkan_>

// |--------------------------------------------------
// ----------------> INSTANCE
// |--------------------------------------------------


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
	
	VkQueue _graphicsQueue;
	VkQueue _presentQueue;

	std::vector<const char*> extensions = {};
	std::vector<const char*> layers = {};
	
	~lvk_device();

	const lvk_physical_device* physical_device;
	const lvk_instance* instance;

	lvk_swapchain init_swapchain(uint32_t width, uint32_t height);
	
	lvk_command_pool init_command_pool();
	lvk_command_pool init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags);
	
	lvk_render_pass init_render_pass();
	lvk_render_pass init_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform = false);

	lvk_semaphore init_semaphore();
	lvk_fence init_fence(VkFenceCreateFlags flags = 0);

	lvk_shader_module init_shader_module(VkShaderStageFlagBits stage, const char* filename);
	
	lvk_pipeline_layout init_pipeline_layout(const VkPushConstantRange* push_constant_range, uint32_t push_constant_range_count);
	
	lvk_allocator init_allocator();

	void wait_idle() const;
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

	~lvk_command_pool();

	const lvk_instance* instance;
	const lvk_physical_device* physical_device;
	const lvk_device* device;

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
	void end();
	void begin_render_pass(const lvk_framebuffer* framebuffer, const VkClearValue* clear_value, const uint32_t clear_value_count, const VkSubpassContents subpass_contents);
	void end_render_pass();

	~lvk_command_buffer();

	void begin_render_pass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents subpass_contents);

	void begin(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info = VK_NULL_HANDLE);
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
	
	~lvk_framebuffer();

	const lvk_render_pass* render_pass;
	const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


struct lvk_semaphore {
	VkSemaphore _semaphore;

	const lvk_device* device;
	
	~lvk_semaphore();
};


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


struct lvk_fence {
	VkFence _fence;

	const lvk_device* device;

	VkResult wait(uint64_t timeout = LVK_TIMEOUT);
	VkResult reset();

	~lvk_fence();
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


// // |--------------------------------------------------
// // ----------------> SHADERS
// // |--------------------------------------------------


// struct lvk_graphics_shaders {
// 	VkShaderModule _vertex_shader = VK_NULL_HANDLE;
// 	VkShaderModule _testallation_control_shader = VK_NULL_HANDLE;
// 	VkShaderModule _testallation_evaluation_shader = VK_NULL_HANDLE;
// 	VkShaderModule _geometry_shader = VK_NULL_HANDLE;
// 	VkShaderModule _fragment_shader = VK_NULL_HANDLE;

// 	~lvk_graphics_shaders();

// 	const lvk_device* device;
// };


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


struct lvk_pipeline_layout {
	VkPipelineLayout _pipeline_layout;
	
	~lvk_pipeline_layout();
	
	lvk_pipeline init_graphics_pipeline(const lvk_render_pass* render_pass, const lvk::graphics_pipeline_config* config);
	
	// const lvk_render_pass* render_pass;
	const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> PIPELINE
// |--------------------------------------------------


struct lvk_pipeline {
	VkPipeline _pipeline;
	
	~lvk_pipeline();
	
	const lvk_pipeline_layout* pipeline_layout;
	const lvk_render_pass* render_pass;
	const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> ALLOCATOR
// |--------------------------------------------------


struct lvk_allocator {
	VmaAllocator _allocator;
	
	const lvk_device* device;

	lvk::deletion_queue deletion_queue;
	
	~lvk_allocator();

	lvk_buffer init_buffer(VkBufferUsageFlagBits usage, const void* data, const std::size_t size, const VkSharingMode sharing_mode, const uint32_t* queue_family_indices, uint32_t queue_family_indices_count);

	lvk_buffer init_vertex_buffer(const std::size_t size);
	lvk_buffer init_vertex_buffer(const void* data, const std::size_t size);

	lvk_image init_image(VkFormat format);
};


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------

struct lvk_buffer {
	VkBuffer _buffer;
	VmaAllocation _allocation;
	
	const lvk_allocator* allocator;

	std::size_t allocated_size;
	VkBufferUsageFlagBits usage;

	void upload(const void* vertex_data, const std::size_t vertex_size);
};


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------

struct lvk_image {
	VkImage _image;
	VmaAllocation _allocation;
	
	VkFormat _format;
	
	const lvk_allocator* allocator;
	const lvk_device* device;

	lvk::deletion_queue* deletion_queue;
	
	lvk_image_view init_image_view(VkImageAspectFlags aspect_flag);
};


// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


struct lvk_image_view {
	VkImageView image_view;
	
	const lvk_image* image;
};
