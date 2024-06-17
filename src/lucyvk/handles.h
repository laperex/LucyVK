#pragma once

#include "vk_mem_alloc.h"
#include <memory>
#include <vulkan/vulkan.h>

struct lvk_device;

struct lvk_instance;


struct lvk_queue;

struct lvk_command_buffer;
struct lvk_shader_module;

struct lvk_allocator;


using lvk_device_ptr = std::unique_ptr<lvk_device>;
struct lvk_device_inherit {
	const lvk_device* device = VK_NULL_HANDLE;
};


// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


struct lvk_command_pool {
	VkCommandPool _command_pool;
};


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


struct lvk_pipeline_layout {
	VkPipelineLayout _pipeline_layout;
};


// |--------------------------------------------------
// ----------------> PIPELINE
// |--------------------------------------------------


struct lvk_pipeline {
	VkPipeline _pipeline;
	
	VkPipelineBindPoint type;
};


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


struct lvk_swapchain {
	VkSwapchainKHR _swapchain;
	VkExtent2D _extent;
	VkSurfaceFormatKHR _surface_format;
	VkPresentModeKHR _present_mode;
	VkImageUsageFlags _image_usage;

	uint32_t _image_count;
	VkImage* _images;
	VkImageView* _image_views;
};


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


struct lvk_semaphore {
	VkSemaphore _semaphore;
};


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


struct lvk_fence {
	VkFence _fence;
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET LAYOUT
// |--------------------------------------------------


struct lvk_descriptor_set_layout {
	VkDescriptorSetLayout _descriptor_set_layout;
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR POOL
// |--------------------------------------------------


struct lvk_descriptor_pool {
	VkDescriptorPool _descriptor_pool;
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


struct lvk_descriptor_set {
	VkDescriptorSet _descriptor_set;
};


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


struct lvk_framebuffer {
	VkFramebuffer _framebuffer;
	VkExtent2D _extent;
};


// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


struct lvk_render_pass {
	VkRenderPass _render_pass;
};


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


struct lvk_buffer {
	VkBuffer _buffer;
	VmaAllocation _allocation;
	
	std::size_t _allocated_size;
	VkBufferUsageFlagBits _usage;
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
};


// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


struct lvk_image_view {
	VkImageView _image_view;
};


// |--------------------------------------------------
// ----------------> COMMAND BUFFER IMMEDIATE
// |--------------------------------------------------


struct lvk_immediate_command {
	VkCommandPool _command_pool;
	VkCommandBuffer _command_buffer;
	VkFence _fence;
};
