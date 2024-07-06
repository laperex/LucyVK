#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <map>
#include <set>
#include <typeinfo>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"


struct lvk_destroyer {
	struct delete_element {
		std::vector<void*> data;
		std::size_t type;
	};

	bool flush = false;

	std::map<void*, uint32_t> data_map;
	std::deque<delete_element> delete_queue;
	std::set<uint32_t> deleted_indices_set;
	
	void push(VkCommandPool command_pool);
	void push(VkPipelineLayout pipeline_layout);
	void push(VkPipeline pipeline);
	void push(VkSwapchainKHR swapchain);
	void push(VkSemaphore semaphore);
	void push(VkFence fence);
	void push(VkDescriptorSetLayout descriptor_set_layout);
	void push(VkDescriptorPool descriptor_pool);
	void push(VkFramebuffer framebuffer);
	void push(VkRenderPass render_pass);
	void push(VkImageView image_view);
	void push(VkShaderModule shader_module);
	void push(VkSampler sampler);
	void push(VkCommandBuffer* command_buffers, uint32_t command_buffer_count, VkCommandPool command_pool);
	void push(VkBuffer buffer, VmaAllocation allocation);
	void push(VkImage buffer, VmaAllocation allocation);

	void delete_insert(void* key);
};
