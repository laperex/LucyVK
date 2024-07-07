#include "destroyer.h"
#include "lucyio/logger.h"
#include <cassert>
#include <cstring>

template <typename T>
void _push(lvk_destroyer* destroyer, T data) {
	static_assert(
		std::is_same<T, VkCommandPool>() ||
		std::is_same<T, VkPipelineLayout>() ||
		std::is_same<T, VkPipeline>() ||
		std::is_same<T, VkSwapchainKHR>() ||
		std::is_same<T, VkSemaphore>() ||
		std::is_same<T, VkFence>() ||
		std::is_same<T, VkDescriptorSetLayout>() ||
		std::is_same<T, VkDescriptorPool>() ||
		std::is_same<T, VkFramebuffer>() ||
		std::is_same<T, VkRenderPass>() ||
		std::is_same<T, VkImageView>() ||
		std::is_same<T, VkShaderModule>() ||
		std::is_same<T, VkSampler>(),
		"Error, Invalid Type!"
	);

	destroyer->data_map[data] = destroyer->delete_queue.push_back({
		.data = { data },
		.type = typeid(T).hash_code()
	});
}


template <typename T>
void _push(lvk_destroyer* destroyer, T data, VmaAllocation allocation) {
	static_assert(
		std::is_same<T, VkBuffer>() ||
		std::is_same<T, VkImage>(),
		"Error, Invalid Type!"
	);

	destroyer->data_map[data] = destroyer->delete_queue.push_back({
		.data = { data, allocation },
		.type = typeid(T).hash_code()
	});
}


template <typename T, typename M>
void _push(lvk_destroyer* destroyer, T* data, uint32_t data_count, M pool) {
	static_assert(
		std::is_same<T, VkDescriptorSet>() ||
		std::is_same<T, VkDescriptorPool>() ||
		std::is_same<M, VkCommandBuffer>() ||
		std::is_same<M, VkCommandPool>(),
		"Error, Invalid Type!"
	);

	assert(data_count);

	lvk_destroyer::delete_element element = {
		.data = std::vector<void*>(data_count + 1),
		.type = typeid(VkCommandBuffer).hash_code()
	};

	std::memcpy(element.data.data(), data, data_count * sizeof(VkCommandBuffer));
	element.data[data_count] = pool;

	destroyer->data_map[data[0]] = destroyer->delete_queue.push_back(element);
}


void lvk_destroyer::delete_insert(void* key) {
	if (flush) return;

	deleted_handles_set.insert(key);
	delete_queue.erase(data_map[key]);
	data_map[key]->value.type = 0;
}


void lvk_destroyer::push(VkCommandBuffer* command_buffers, uint32_t command_buffer_count, VkCommandPool command_pool) {
	_push<VkCommandBuffer, VkCommandPool>(this, command_buffers, command_buffer_count, command_pool);
}


void lvk_destroyer::push(VkBuffer buffer, VmaAllocation allocation) {
	_push(this, buffer, allocation);
}

void lvk_destroyer::push(const lvk_buffer& buffer) {
	_push(this, buffer._buffer, buffer._allocation);
}

void lvk_destroyer::push(VkImage image, VmaAllocation allocation) {
	_push(this, image, allocation);
}

void lvk_destroyer::push(const lvk_image& image) {
	_push(this, image._image, image._allocation);
}

void lvk_destroyer::push(VkCommandPool command_pool) {
	_push(this, command_pool);
}

void lvk_destroyer::push(VkPipelineLayout pipeline_layout) {
	_push(this, pipeline_layout);
}

void lvk_destroyer::push(VkPipeline pipeline) {
	_push(this, pipeline);
}

void lvk_destroyer::push(VkSwapchainKHR swapchain) {
	_push(this, swapchain);
}

void lvk_destroyer::push(VkSemaphore semaphore) {
	_push(this, semaphore);
}

void lvk_destroyer::push(VkFence fence) {
	_push(this, fence);
}

void lvk_destroyer::push(VkDescriptorSetLayout descriptor_set_layout) {
	_push(this, descriptor_set_layout);
}

void lvk_destroyer::push(VkDescriptorPool descriptor_pool) {
	_push(this, descriptor_pool);
}

void lvk_destroyer::push(VkFramebuffer framebuffer) {
	_push(this, framebuffer);
}

void lvk_destroyer::push(VkRenderPass render_pass) {
	_push(this, render_pass);
}

void lvk_destroyer::push(VkImageView image_view) {
	_push(this, image_view);
}

void lvk_destroyer::push(VkShaderModule shader_module) {
	_push(this, shader_module);
}

void lvk_destroyer::push(VkSampler sampler) {
	_push(this, sampler);
}

