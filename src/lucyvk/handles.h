#pragma once

#include "vk_mem_alloc.h"
#include <vector>
#include <vulkan/vulkan.h>
#include "lucyvk/define.h"



struct lvk_device;
struct lvk_instance;


struct lvk_command_buffer;


// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


struct lvk_command_pool {
	LVK_HANDLE_DEF(VkCommandPool, _command_pool)
};


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


struct lvk_pipeline_layout {
	LVK_HANDLE_DEF(VkPipelineLayout, _pipeline_layout)
};


// |--------------------------------------------------
// ----------------> PIPELINE
// |--------------------------------------------------


struct lvk_pipeline {
	LVK_HANDLE_DEF(VkPipeline, _pipeline)
};


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


struct lvk_swapchain {
	LVK_HANDLE_DEF(VkSwapchainKHR, _swapchain)
	LVK_HANDLE_DEF(VkExtent2D, _extent)

	VkSurfaceFormatKHR _surface_format;
	VkPresentModeKHR _present_mode;
	VkImageUsageFlags _image_usage;

	uint32_t _image_count;
	std::vector<VkFramebuffer> _framebuffers;
	std::vector<VkImageView> _image_views;

	VkImageView _depth_image_view;
};


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


struct lvk_semaphore {
	LVK_HANDLE_DEF(VkSemaphore, _semaphore)
};


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


struct lvk_fence {
	LVK_HANDLE_DEF(VkFence, _fence)
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET LAYOUT
// |--------------------------------------------------


struct lvk_descriptor_set_layout {
	LVK_HANDLE_DEF(VkDescriptorSetLayout, _descriptor_set_layout)
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR POOL
// |--------------------------------------------------


struct lvk_descriptor_pool {
	LVK_HANDLE_DEF(VkDescriptorPool, _descriptor_pool)
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


struct lvk_descriptor_set {
	LVK_HANDLE_DEF(VkDescriptorSet, _descriptor_set)
};


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


struct lvk_framebuffer {
	LVK_HANDLE_DEF(VkFramebuffer, _framebuffer)
	VkExtent2D _extent;
};


// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


struct lvk_render_pass {
	LVK_HANDLE_DEF(VkRenderPass, _render_pass)
};


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


struct lvk_buffer {
	LVK_HANDLE_DEF(VkBuffer, _buffer)
	VmaAllocation _allocation;

	VkDeviceSize _allocated_size;
	VkBufferUsageFlags _usage;
	VmaMemoryUsage _memory_usage;
};


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------


struct lvk_image {
	LVK_HANDLE_DEF(VkImage, _image)

	VmaAllocation _allocation;
	
	VkFormat _format;
	VkImageType _image_type;
	VkExtent3D _extent;
	VkImageUsageFlags _usage;
	VmaMemoryUsage _memory_usage;
};


// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


struct lvk_image_view {
	LVK_HANDLE_DEF(VkImageView, _image_view)
};


// |--------------------------------------------------
// ----------------> SAMPLER
// |--------------------------------------------------


struct lvk_shader_module {
	LVK_HANDLE_DEF(VkShaderModule, _shader_module)

	// const VkShaderStageFlagBits _stage;
};


// |--------------------------------------------------
// ----------------> SAMPLER
// |--------------------------------------------------


struct lvk_sampler {
	LVK_HANDLE_DEF(VkSampler, _sampler)
};
