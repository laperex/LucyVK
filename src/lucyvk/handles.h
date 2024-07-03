#pragma once

#include "vk_mem_alloc.h"
#include <memory>
#include <vulkan/vulkan.h>

#define HANDLE_DEF(h_type, h_name) h_type h_name; operator decltype(h_name)() const { return h_name; }

struct lvk_device;

struct lvk_instance;


struct lvk_queue;

struct lvk_command_buffer;
struct lvk_shader_module;

struct lvk_allocator;


// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


struct lvk_command_pool {
	HANDLE_DEF(VkCommandPool, _command_pool)
};


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


struct lvk_pipeline_layout {
	HANDLE_DEF(VkPipelineLayout, _pipeline_layout)
};


// |--------------------------------------------------
// ----------------> PIPELINE
// |--------------------------------------------------


struct lvk_pipeline {
	HANDLE_DEF(VkPipeline, _pipeline)
	
	// VkPipelineBindPoint type;
};


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


struct lvk_swapchain {
	HANDLE_DEF(VkSwapchainKHR, _swapchain)
	HANDLE_DEF(VkExtent2D, _extent)

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
	HANDLE_DEF(VkSemaphore, _semaphore)
};


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


struct lvk_fence {
	HANDLE_DEF(VkFence, _fence)
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET LAYOUT
// |--------------------------------------------------


struct lvk_descriptor_set_layout {
	HANDLE_DEF(VkDescriptorSetLayout, _descriptor_set_layout)
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR POOL
// |--------------------------------------------------


struct lvk_descriptor_pool {
	HANDLE_DEF(VkDescriptorPool, _descriptor_pool)
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


struct lvk_descriptor_set {
	HANDLE_DEF(VkDescriptorSet, _descriptor_set)
};


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


struct lvk_framebuffer {
	HANDLE_DEF(VkFramebuffer, _framebuffer)
	VkExtent2D _extent;
};


// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


struct lvk_render_pass {
	HANDLE_DEF(VkRenderPass, _render_pass)
};


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


struct lvk_buffer {
	HANDLE_DEF(VkBuffer, _buffer)

	VmaAllocation _allocation;
	
	std::size_t _allocated_size;
	VkBufferUsageFlagBits _usage;
};


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------


struct lvk_image {
	HANDLE_DEF(VkImage, _image)

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
	HANDLE_DEF(VkImageView, _image_view)
};


// |--------------------------------------------------
// ----------------> SAMPLER
// |--------------------------------------------------


struct lvk_sampler {
	HANDLE_DEF(VkSampler, _sampler)
};


// |--------------------------------------------------
// ----------------> COMMAND BUFFER IMMEDIATE
// |--------------------------------------------------


struct lvk_immediate_command {
	HANDLE_DEF(VkCommandPool, _command_pool)
	HANDLE_DEF(VkCommandBuffer, _command_buffer)
	HANDLE_DEF(VkFence, _fence)
};