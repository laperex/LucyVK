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
	struct delete_element {
		std::vector<void*> data = {};
		std::size_t type = 0;
	};
	
	typedef std::function<void()> void_lambda_function;

	std::deque<delete_element> delete_deque;
	VkDevice device;
	VmaAllocator allocator;

private:
	void push_function(VkCommandPool command_pool);
	void push_function(VkPipelineLayout pipeline_layout);
	void push_function(VkPipeline pipeline);
	void push_function(VkSwapchainKHR swapchain);
	void push_function(VkSemaphore semaphore);
	void push_function(VkFence fence);
	void push_function(VkDescriptorSetLayout descriptor_set_layout);
	void push_function(VkDescriptorPool descriptor_pool);
	void push_function(VkFramebuffer framebuffer);
	void push_function(VkRenderPass render_pass);
	void push_function(VkImageView image_view);
	void push_function(VkShaderModule shader_module);
	void push_function(VkSampler sampler);
	void push_function(VkCommandBuffer* command_buffers, uint32_t command_buffer_count, VkCommandPool command_pool);
	void push_function(VkDescriptorSet* descriptor_sets, uint32_t descriptor_set_count, VkDescriptorPool descriptor_pool);
	// void push_function(VkCommandBuffer command_buffers, uint32_t command_buffer_count, VkCommandPool command_pool);

	void push_function(VkBuffer buffer, VmaAllocation allocation);
	void push_function(VkImage buffer, VmaAllocation allocation);

public:
	const lvk_command_buffer& push(const lvk_command_buffer& command_buffer, const lvk_command_pool& command_pool);
	const lvk_descriptor_set& push(const lvk_descriptor_set& descriptor_set, const lvk_descriptor_pool& descriptor_pool);
	const lvk_buffer& push(const lvk_buffer& buffer);
	const lvk_image& push(const lvk_image& image);

	template <typename T>
	const T& push(const T& handle) {
		static_assert(
			std::is_same<T, lvk_command_buffer>() ||
			std::is_same<T, lvk_command_pool>() ||
			std::is_same<T, lvk_pipeline_layout>() ||
			std::is_same<T, lvk_pipeline>() ||
			std::is_same<T, lvk_semaphore>() ||
			std::is_same<T, lvk_fence>() ||
			std::is_same<T, lvk_descriptor_set_layout>() ||
			std::is_same<T, lvk_descriptor_pool>() ||
			std::is_same<T, lvk_framebuffer>() ||
			std::is_same<T, lvk_render_pass>() ||
			std::is_same<T, lvk_image_view>() ||
			std::is_same<T, lvk_shader_module>() ||
			std::is_same<T, lvk_sampler>(),
			"Error, Invalid Type!"
		);
		push_function(handle);
		return handle;
	}
	
	void push_fn(std::function<void()>&& function);
	
	void flush();

	// void destroy(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool);

	void destroy(VkCommandPool command_pool);
	void destroy(VkPipelineLayout pipeline_layout);
	void destroy(VkPipeline pipeline);
	void destroy(VkSwapchainKHR swapchain);
	void destroy(VkSemaphore semaphore);
	void destroy(VkFence fence);
	void destroy(VkDescriptorSetLayout descriptor_set_layout);
	void destroy(VkDescriptorPool descriptor_pool);
	void destroy(VkFramebuffer framebuffer);
	void destroy(VkRenderPass render_pass);
	void destroy(VkImageView image_view);
	void destroy(VkShaderModule shader_module);
	void destroy(VkSampler sampler);
	// void destroy(VkCommandBuffer command_buffer, VkCommandPool command_pool);
	void destroy(VkCommandBuffer* command_buffer, uint32_t command_buffer_count, VkCommandPool command_pool);
	void destroy(VkBuffer buffer, VmaAllocation allocation);
	void destroy(const lvk_buffer& buffer);
	void destroy(VkImage image, VmaAllocation allocation);
	void destroy(const lvk_image& image);

	// void destroy(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool);

	void destroy(VkDescriptorSet* descriptor_set, uint32_t descriptor_set_count, VkDescriptorPool descriptor_pool);
};
