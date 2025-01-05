#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <typeinfo>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "lucyvk/handles.h"
#include "vk_mem_alloc.h"
#include "lucytl/linked_list.h"


// TODO better deletion queue
// struct lvk_destroyer {
// 	struct delete_element {
// 		std::vector<void*> data;
// 		std::size_t type;
// 	};

// 	bool flush = false;

// 	std::map<void*, ltl::linked_list<delete_element>::node*> data_map;
// 	ltl::linked_list<delete_element> delete_queue;
// 	std::set<void*> deleted_handles_set;

// 	void push(VkCommandPool command_pool);
// 	void push(VkPipelineLayout pipeline_layout);
// 	void push(VkPipeline pipeline);
// 	void push(VkSwapchainKHR swapchain);
// 	void push(VkSemaphore semaphore);
// 	void push(VkFence fence);
// 	void push(VkDescriptorSetLayout descriptor_set_layout);
// 	void push(VkDescriptorPool descriptor_pool);
// 	void push(VkFramebuffer framebuffer);
// 	void push(VkRenderPass render_pass);
// 	void push(VkImageView image_view);
// 	void push(VkShaderModule shader_module);
// 	void push(VkSampler sampler);
// 	void push(VkCommandBuffer* command_buffers, uint32_t command_buffer_count, VkCommandPool command_pool);

// 	void push(VkBuffer buffer, VmaAllocation allocation);
// 	void push(const lvk_buffer& buffer);
// 	void push(VkImage buffer, VmaAllocation allocation);
// 	void push(const lvk_image& image);

// 	void delete_insert(void* key);
// };


struct lvk_deletor_deque {
	std::deque<std::function<void()>> delete_deque;
	VkDevice device;
	VmaAllocator allocator;

	const lvk_command_buffer& push(const lvk_command_buffer& command_buffer, const lvk_command_pool& command_pool);
	const lvk_descriptor_set& push(const lvk_descriptor_set& descriptor_set, const lvk_descriptor_pool& descriptor_pool);
	const lvk_buffer& push(const lvk_buffer& buffer);
	const lvk_image& push(const lvk_image& image);
	
	

	#define PUSH_DEF(T, M) \
		const T& push(const T& handle) { \
			push_fn([=, this]() { \
				destroy(static_cast<M>(handle)); \
			}); \
			return handle; \
		}

	PUSH_DEF(lvk_command_pool, VkCommandPool);
	PUSH_DEF(lvk_pipeline_layout, VkPipelineLayout);
	PUSH_DEF(lvk_pipeline, VkPipeline);
	PUSH_DEF(lvk_semaphore, VkSemaphore);
	PUSH_DEF(lvk_fence, VkFence);
	PUSH_DEF(lvk_descriptor_set_layout, VkDescriptorSetLayout);
	PUSH_DEF(lvk_descriptor_pool, VkDescriptorPool);
	PUSH_DEF(lvk_framebuffer, VkFramebuffer);
	PUSH_DEF(lvk_render_pass, VkRenderPass);
	PUSH_DEF(lvk_image_view, VkImageView);
	PUSH_DEF(lvk_shader_module, VkShaderModule);
	PUSH_DEF(lvk_sampler, VkSampler);
	
	void push_fn(std::function<void()>&& function);
	
	void flush();

	// void destroy(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool);

	void destroy(VkCommandPool command_pool) const;
	void destroy(VkPipelineLayout pipeline_layout) const;
	void destroy(VkPipeline pipeline) const;
	void destroy(VkSwapchainKHR swapchain) const;
	void destroy(VkSemaphore semaphore) const;
	void destroy(VkFence fence) const;
	void destroy(VkDescriptorSetLayout descriptor_set_layout) const;
	void destroy(VkDescriptorPool descriptor_pool) const;
	void destroy(VkFramebuffer framebuffer) const;
	void destroy(VkRenderPass render_pass) const;
	void destroy(VkImageView image_view) const;
	void destroy(VkShaderModule shader_module) const;
	void destroy(VkSampler sampler) const;
	
	void destroy(VkCommandBuffer* command_buffer, uint32_t command_buffer_count, VkCommandPool command_pool) const;
	void destroy(VkCommandBuffer command_buffer, VkCommandPool command_pool) const;
	void destroy(VkBuffer buffer, VmaAllocation allocation) const;
	void destroy(const lvk_buffer& buffer) const;
	void destroy(VkImage image, VmaAllocation allocation) const;
	void destroy(const lvk_image& image) const;

	// void destroy(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool);

	void destroy(VkDescriptorSet* descriptor_set, uint32_t descriptor_set_count, VkDescriptorPool descriptor_pool) const;
};
