#pragma once

#include <SDL_video.h>
#include <functional>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

#define LVK_EXCEPT(assertion, message)	\
	if ((assertion) != true)	\
		throw std::runtime_error(message);


#define LVK_TIMEOUT 1000000000

struct lvk_instance;
struct lvk_physical_device;
struct lvk_device;
struct lvk_command_pool;
struct lvk_command_buffer;
struct lvk_swapchain;
struct lvk_render_pass;
struct lvk_framebuffer;
struct lvk_semaphore;
struct lvk_fence;
struct lvk_shader_module;
struct lvk_pipeline;


enum LVK_SHADER_STAGE_FLAG {
	LVK_SHADER_STAGE_VERTEX = 0x00000001,					// VK_SHADER_STAGE_VERTEX_BIT,
	LVK_SHADER_STAGE_TESSELLATION_CONTROL = 0x00000002,		// VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
	LVK_SHADER_STAGE_TESSELLATION_EVALUATION = 0x00000004,	// VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
	LVK_SHADER_STAGE_GEOMETRY = 0x00000008,					// VK_SHADER_STAGE_GEOMETRY_BIT,
	LVK_SHADER_STAGE_FRAGMENT = 0x00000010,					// VK_SHADER_STAGE_FRAGMENT_BIT,
	LVK_SHADER_STAGE_COMPUTE = 0x00000020,					// VK_SHADER_STAGE_COMPUTE_BIT,
	LVK_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,				// VK_SHADER_STAGE_ALL_GRAPHICS,
};


namespace lvk {
	struct queue_family_indices {
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;
		std::optional<uint32_t> compute;
		std::optional<uint32_t> sparse_binding;

		operator bool() const {
			return graphics.has_value() && present.has_value();
		}

		const bool unique() const {
			return (graphics.value() == present.value());
		}
	};

	struct swapchain_support_details {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	typedef std::function<VkPhysicalDevice(const std::vector<VkPhysicalDevice>&, const lvk_instance* instance)> SelectPhysicalDeviceFunction;

	lvk_instance initialize(const char* name, SDL_Window* sdl_window, bool debug_enable);
}


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
	void destroy_swapchain(lvk_swapchain* swapchain);
	
	lvk_command_pool init_command_pool();
	lvk_command_pool init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags);
	
	lvk_render_pass init_render_pass();
	lvk_render_pass init_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform = false);
	
	lvk_semaphore init_semaphore(const uint32_t count, VkSemaphoreCreateFlags flags = 0);
	lvk_fence init_fence(const uint32_t count, VkFenceCreateFlags flags = 0);
	
	lvk_pipeline init_pipeline();
	
	void wait_idle();

	lvk_shader_module init_shader_module(LVK_SHADER_STAGE_FLAG stage, const char* filename);

	std::vector<VkPipelineShaderStageCreateInfo> create_shader_stage_array(std::vector<lvk_shader_module> shader_module);
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
	
	uint32_t acquire_next_image(const uint64_t timeout = LVK_TIMEOUT, VkSemaphore semaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE);
	
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
	void destroy_framebuffer(lvk_framebuffer* framebuffer);
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


// |--------------------------------------------------
// ----------------> SHADER
// |--------------------------------------------------

struct lvk_shader_module {
	VkShaderModule _shader_module;
	const LVK_SHADER_STAGE_FLAG stage;

	~lvk_shader_module();

	const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> PIPELINE
// |--------------------------------------------------


struct lvk_pipeline {
	VkPipeline _pipeline;
	
	~lvk_pipeline();
	
	const lvk_device* device;
};

