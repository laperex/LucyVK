#include "destroyer.h"
#include "command.h"
#include "lucyio/logger.h"
#include <cassert>
#include <cstring>

template <typename T>
void _push(lvk_deletor_deque* destroyer, T data) {
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

	destroyer->delete_deque.push_back({
		.data = { data },
		.type = typeid(T).hash_code()
	});
}


template <typename T>
void _push(lvk_deletor_deque* destroyer, T data, VmaAllocation allocation) {
	static_assert(
		std::is_same<T, VkBuffer>() ||
		std::is_same<T, VkImage>(),
		"Error, Invalid Type!"
	);

	destroyer->delete_deque.push_back({
		.data = { data, allocation },
		.type = typeid(T).hash_code()
	});
}


template <typename T, typename M>
void _push(lvk_deletor_deque* destroyer, T* data, uint32_t data_count, M pool) {
	static_assert(
		(std::is_same<T, VkDescriptorSet>() && std::is_same<M, VkDescriptorPool>()) ||
		(std::is_same<T, VkCommandBuffer>() && std::is_same<M, VkCommandPool>()),
		"Error, Invalid Type!"
	);

	assert(data_count);

	lvk_deletor_deque::delete_element element = {
		.data = std::vector<void*>(data_count + 1),
		.type = typeid(T).hash_code()
	};

	std::memcpy(element.data.data(), data, data_count * sizeof(T));
	element.data[data_count] = pool;

	destroyer->delete_deque.push_back(element);
}


void lvk_deletor_deque::push_function(VkCommandBuffer* command_buffers, uint32_t command_buffer_count, VkCommandPool command_pool) {
	_push<VkCommandBuffer, VkCommandPool>(this, command_buffers, command_buffer_count, command_pool);
}

void lvk_deletor_deque::push_function(VkDescriptorSet* descriptor_sets, uint32_t descriptor_set_count, VkDescriptorPool descriptor_pool) {
	_push<VkDescriptorSet, VkDescriptorPool>(this, descriptor_sets, descriptor_set_count, descriptor_pool);
}


void lvk_deletor_deque::push_function(VkBuffer buffer, VmaAllocation allocation) {
	_push(this, buffer, allocation);
}

void lvk_deletor_deque::push_function(VkImage image, VmaAllocation allocation) {
	_push(this, image, allocation);
}

const lvk_command_buffer& lvk_deletor_deque::push(const lvk_command_buffer& command_buffer, const lvk_command_pool& command_pool) {
	push_function((VkCommandBuffer*)&command_buffer, 1, command_pool);
	return command_buffer;
}

const lvk_descriptor_set& lvk_deletor_deque::push(const lvk_descriptor_set& descriptor_set, const lvk_descriptor_pool& descriptor_pool) {
	push_function((VkDescriptorSet*)&descriptor_set, 1, descriptor_pool);
	return descriptor_set;
}

void lvk_deletor_deque::push_function(VkCommandPool command_pool) {
	_push(this, command_pool);
}

void lvk_deletor_deque::push_function(VkPipelineLayout pipeline_layout) {
	_push(this, pipeline_layout);
}

void lvk_deletor_deque::push_function(VkPipeline pipeline) {
	_push(this, pipeline);
}

void lvk_deletor_deque::push_function(VkSwapchainKHR swapchain) {
	_push(this, swapchain);
}

void lvk_deletor_deque::push_function(VkSemaphore semaphore) {
	_push(this, semaphore);
}

void lvk_deletor_deque::push_function(VkFence fence) {
	_push(this, fence);
}

void lvk_deletor_deque::push_function(VkDescriptorSetLayout descriptor_set_layout) {
	_push(this, descriptor_set_layout);
}

void lvk_deletor_deque::push_function(VkDescriptorPool descriptor_pool) {
	_push(this, descriptor_pool);
}

void lvk_deletor_deque::push_function(VkFramebuffer framebuffer) {
	_push(this, framebuffer);
}

void lvk_deletor_deque::push_function(VkRenderPass render_pass) {
	_push(this, render_pass);
}

void lvk_deletor_deque::push_function(VkImageView image_view) {
	_push(this, image_view);
}

void lvk_deletor_deque::push_function(VkShaderModule shader_module) {
	_push(this, shader_module);
}

void lvk_deletor_deque::push_function(VkSampler sampler) {
	_push(this, sampler);
}


const lvk_buffer& lvk_deletor_deque::push(const lvk_buffer& buffer) {
	_push(this, buffer._buffer, buffer._allocation);
	return buffer;
}

const lvk_image& lvk_deletor_deque::push(const lvk_image& image) {
	_push(this, image._image, image._allocation);
	return image;
}


void lvk_deletor_deque::flush() {
	int i = 0;
	for (auto element = delete_deque.rbegin(); element != delete_deque.rend(); element++) {
		if (element->type == typeid(VkCommandPool).hash_code()) {
			destroy(static_cast<VkCommandPool>(element->data[0]));
		}
		if (element->type == typeid(VkPipelineLayout).hash_code()) {
			destroy(static_cast<VkPipelineLayout>(element->data[0]));
		}
		if (element->type == typeid(VkPipeline).hash_code()) {
			destroy(static_cast<VkPipeline>(element->data[0]));
		}
		if (element->type == typeid(VkSwapchainKHR).hash_code()) {
			destroy(static_cast<VkSwapchainKHR>(element->data[0]));
		}
		if (element->type == typeid(VkSemaphore).hash_code()) {
			destroy(static_cast<VkSemaphore>(element->data[0]));
		}
		if (element->type == typeid(VkFence).hash_code()) {
			destroy(static_cast<VkFence>(element->data[0]));
		}
		if (element->type == typeid(VkDescriptorSetLayout).hash_code()) {
			destroy(static_cast<VkDescriptorSetLayout>(element->data[0]));
		}
		if (element->type == typeid(VkDescriptorPool).hash_code()) {
			destroy(static_cast<VkDescriptorPool>(element->data[0]));
		}
		if (element->type == typeid(VkFramebuffer).hash_code()) {
			destroy(static_cast<VkFramebuffer>(element->data[0]));
		}
		if (element->type == typeid(VkRenderPass).hash_code()) {
			destroy(static_cast<VkRenderPass>(element->data[0]));
		}
		if (element->type == typeid(VkImageView).hash_code()) {
			destroy(static_cast<VkImageView>(element->data[0]));
		}
		if (element->type == typeid(VkShaderModule).hash_code()) {
			destroy(static_cast<VkShaderModule>(element->data[0]));
		}
		if (element->type == typeid(VkSampler).hash_code()) {
			destroy(static_cast<VkSampler>(element->data[0]));
		}

		if (element->type == typeid(VkCommandBuffer).hash_code()) {
			destroy((VkCommandBuffer*)element->data.data(), element->data.size() - 1, (VkCommandPool)element->data.back());
		}
		
		if (element->type == typeid(VkDescriptorSet).hash_code()) {
			destroy((VkDescriptorSet*)element->data.data(), element->data.size() - 1, (VkDescriptorPool)element->data.back());
		}

		if (element->type == typeid(VkBuffer).hash_code()) {
			destroy((VkBuffer)element->data[0], (VmaAllocation)element->data[1]);
		}
		
		if (element->type == typeid(VkImage).hash_code()) {
			destroy((VkImage)element->data[0], (VmaAllocation)element->data[1]);
		}
	}

	delete_deque.clear();
}


void lvk_deletor_deque::destroy(VkCommandPool command_pool) {
	vkDestroyCommandPool(device, command_pool, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", command_pool, "\t [CommandPool]");
	// deletor.push_function()
}


void lvk_deletor_deque::destroy(VkPipelineLayout pipeline_layout) {
	vkDestroyPipelineLayout(device, pipeline_layout, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", pipeline_layout, "\t [PipelineLayout]");
}


void lvk_deletor_deque::destroy(VkPipeline pipeline) {
	vkDestroyPipeline(device, pipeline, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", pipeline, "\t [Pipeline]");
}


void lvk_deletor_deque::destroy(VkSwapchainKHR swapchain) {
	vkDestroySwapchainKHR(device, swapchain, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", swapchain, "\t [SwapchainKHR]");
}


void lvk_deletor_deque::destroy(VkSemaphore semaphore) {
	vkDestroySemaphore(device, semaphore, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", semaphore, "\t [Semaphore]");
}


void lvk_deletor_deque::destroy(VkFence fence) {
	vkDestroyFence(device, fence, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", fence, "\t [Fence]");
}




// void lvk_deletor_deque::destroy(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool) {
// 	vkFreeDescriptorSets(device, descriptor_pool, 1, &descriptor_set);
// 	dloggln("DESTROYED \t", descriptor_set, "\t [DescriptionSet]");
// }

void lvk_deletor_deque::destroy(VkDescriptorSet* descriptor_set, uint32_t descriptor_set_count, VkDescriptorPool descriptor_pool) {
	assert(descriptor_set_count);

	vkFreeDescriptorSets(device, descriptor_pool, descriptor_set_count, descriptor_set);
	dloggln("DESTROYED \t", descriptor_set[0], "\t [DescriptionSet]");
}


void lvk_deletor_deque::destroy(VkDescriptorSetLayout descriptor_set_layout) {
	vkDestroyDescriptorSetLayout(device, descriptor_set_layout, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", descriptor_set_layout, "\t [DescriptorSetLayout]");
}


void lvk_deletor_deque::destroy(VkDescriptorPool descriptor_pool) {
	vkDestroyDescriptorPool(device, descriptor_pool, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", descriptor_pool, "\t [DescriptorPool]");
}


void lvk_deletor_deque::destroy(VkFramebuffer framebuffer) {
	dloggln("[ERR] \t", framebuffer, "\t [Framebuffer]");
	vkDestroyFramebuffer(device, framebuffer, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", framebuffer, "\t [Framebuffer]");
}


void lvk_deletor_deque::destroy(VkRenderPass render_pass) {
	vkDestroyRenderPass(device, render_pass, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", render_pass, "\t [RenderPass]");
}


void lvk_deletor_deque::destroy(VkImageView image_view) {
	if (image_view == VK_NULL_HANDLE) { return; }

	vkDestroyImageView(device, image_view, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", image_view, "\t [ImageView]");
}


void lvk_deletor_deque::destroy(VkShaderModule shader_module) {
	vkDestroyShaderModule(device, shader_module, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", shader_module, "\t [ShaderModule]");
}


void lvk_deletor_deque::destroy(VkSampler sampler) {
	vkDestroySampler(device, sampler, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", sampler, "\t [Sampler]");
}


// void lvk_deletor_deque::destroy(VkCommandBuffer command_buffer, VkCommandPool command_pool) {
// 	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
// 	dloggln("DESTROYED \t", command_buffer, "\t [CommandBuffer]");
// }

void lvk_deletor_deque::destroy(VkCommandBuffer* command_buffer, uint32_t command_buffer_count, VkCommandPool command_pool) {
	assert(command_buffer_count);

	vkFreeCommandBuffers(device, command_pool, command_buffer_count, command_buffer);
	dloggln("DESTROYED \t", command_buffer[0], "\t [CommandBuffer]");
}

void lvk_deletor_deque::destroy(VkBuffer buffer, VmaAllocation allocation) {
	vmaDestroyBuffer(allocator, buffer, allocation);
	dloggln("DESTROYED \t", buffer, "\t [Buffer]");
}

void lvk_deletor_deque::destroy(const lvk_buffer& buffer) {
	destroy(buffer, buffer._allocation);
}

void lvk_deletor_deque::destroy(VkImage image, VmaAllocation allocation) {
	vmaDestroyImage(allocator, image, allocation);
	dloggln("DESTROYED \t", image, "\t [Image]");
}

void lvk_deletor_deque::destroy(const lvk_image& image) {
	destroy(image, image._allocation);
}