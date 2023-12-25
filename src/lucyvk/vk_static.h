#pragma once

#include "vk_types.h"

#define VMA_IMPLEMENTATION

#include <SDL_video.h>
#include <functional>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>


// |--------------------------------------------------
// ----------------> INSTANCE
// |--------------------------------------------------


struct lvk_instance {
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkSurfaceKHR _surface;
	VkInstance _instance;

	// lvk::deletion_queue deletion_queue;

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

	lvk_device init_device();

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

	std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	
	~lvk_device();

	const lvk_instance* instance;
	const lvk_physical_device* physical_device;

	uint32_t swapchain_count = 0;	
	lvk_swapchain* create_swapchain(uint32_t width, uint32_t height);
	
	lvk_command_pool init_command_pool();
	lvk_command_pool init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags);
	
	lvk_render_pass init_render_pass();
	lvk_render_pass init_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform = false);
	
	lvk_semaphore init_semaphore(const uint32_t count, VkSemaphoreCreateFlags flags = 0);
	lvk_fence init_fence(const uint32_t count, VkFenceCreateFlags flags = 0);
	
// 	lvk_graphics_pipeline init_graphics_pipeline(
// 	const VkPipelineShaderStageCreateInfo* shader_stage, const uint32_t shader_stage_count,
// 	const VkPipelineVertexInputStateCreateInfo*      vertex_input_state,
// 	const VkPipelineInputAssemblyStateCreateInfo*    input_assembly_state,
// 	const VkPipelineTessellationStateCreateInfo*     tessellation_state,
// 	const VkPipelineViewportStateCreateInfo*         viewport_state,
// 	const VkPipelineRasterizationStateCreateInfo*    rasterization_state,
// 	const VkPipelineMultisampleStateCreateInfo*      multisample_state,
// 	const VkPipelineDepthStencilStateCreateInfo*     depth_stencil_state,
// 	const VkPipelineColorBlendStateCreateInfo*       color_blend_state,
// 	const VkPipelineDynamicStateCreateInfo*          dynamic_state
// );
	lvk_shader_module init_shader_module(VkShaderStageFlagBits stage, const char* filename);
	
	lvk_pipeline_layout init_pipeline_layout();

	void wait_idle();
};


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


struct lvk_swapchain {
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	VkExtent2D _extent = { 0, 0 };
	VkSurfaceFormatKHR _surface_format;
	VkPresentModeKHR _present_mode;

	std::vector<VkImage> _images;
	std::vector<VkImageView> _image_view_array;
	
	uint32_t acquire_next_image(VkSemaphore semaphore, VkFence fence, const uint64_t timeout = LVK_TIMEOUT);
	
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

	lvk_command_buffer init_command_buffer(uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
};


// |--------------------------------------------------
// ----------------> COMMAND BUFFER
// |--------------------------------------------------


struct lvk_command_buffer {
	VkCommandBuffer* _command_buffers;

	const uint32_t _count;

	const lvk_instance* instance;
	const lvk_physical_device* physical_device;
	const lvk_device* device;
	const lvk_command_pool* command_pool;

	void reset(const uint32_t index = 0, VkCommandBufferResetFlags flags = 0);
	void reset_all(VkCommandBufferResetFlags flags = 0);
	void cmd_begin(const uint32_t index, const VkCommandBufferBeginInfo* beginInfo);
	void cmd_end(const uint32_t index);
	void cmd_render_pass_begin(const uint32_t index, const VkRenderPassBeginInfo* beginInfo, VkSubpassContents subpass_contents);
	void cmd_render_pass_end(const uint32_t index);

	~lvk_command_buffer();
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

	// VkRenderPassBeginInfo begin_info();

	lvk_framebuffer* create_framebuffer(uint32_t width, uint32_t height, const std::vector<VkImageView>& image_view_array);
};


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


struct lvk_semaphore {
	VkSemaphore* _semaphore;
	const uint32_t _count;

	const lvk_device* device;
	
	~lvk_semaphore();
};


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


struct lvk_fence {
	VkFence* _fence;
	const uint32_t _count;

	const lvk_device* device;

	VkResult wait(bool wait_all = true, uint64_t timeout = LVK_TIMEOUT);
	VkResult reset();

	~lvk_fence();
};


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


struct lvk_framebuffer {
	std::vector<VkFramebuffer> _framebuffer_array;
	
	~lvk_framebuffer();

	const lvk_render_pass* render_pass;
	const lvk_device* device;
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

