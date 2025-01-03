#include "destroyer.h"
#include "command.h"
#include "lucyio/logger.h"
#include <cassert>
#include <cstring>


const lvk_command_buffer& lvk_deletor_deque::push(const lvk_command_buffer& command_buffer, const lvk_command_pool& command_pool) {
	push_fn([=, this]() {
		destroy((VkCommandBuffer*)&command_buffer, 1, command_pool);
	});
	return command_buffer;
}

const lvk_descriptor_set& lvk_deletor_deque::push(const lvk_descriptor_set& descriptor_set, const lvk_descriptor_pool& descriptor_pool) {
	push_fn([=, this]() {
		destroy((VkDescriptorSet*)&descriptor_set, 1, descriptor_pool);
	});
	return descriptor_set;
}

const lvk_buffer& lvk_deletor_deque::push(const lvk_buffer& buffer) {
	push_fn([=, this]() {
		destroy((VkBuffer)buffer._buffer, (VmaAllocation)buffer._allocation);
	});
	return buffer;
}

const lvk_image& lvk_deletor_deque::push(const lvk_image& image) {
	push_fn([=, this]() {
		destroy((VkImage)image._image, (VmaAllocation)image._allocation);
	});
	return image;
}

void lvk_deletor_deque::push_fn(std::function<void()>&& function) {
	delete_deque.push_back(function);
}

void lvk_deletor_deque::flush() {
	for (auto element = delete_deque.rbegin(); element != delete_deque.rend(); element++) {
		(*element)();
	}

	delete_deque.clear();
}


void lvk_deletor_deque::destroy(VkCommandPool command_pool) {
	vkDestroyCommandPool(device, command_pool, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", command_pool, "\t [CommandPool]");
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