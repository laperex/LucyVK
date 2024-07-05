#include "handles.h"
#include <cassert>
#include <lucyio/logger.h>

void lvk_destroy(VkDevice device) {
	vkDestroyDevice(device, VK_NULL_HANDLE);
	dloggln("Destroyed: ", device, "\t [LogicalDevice]");
}

void lvk_destroy(VkDevice device, VkCommandPool command_pool) {
	vkDestroyCommandPool(device, command_pool, VK_NULL_HANDLE);
	dloggln("Destroyed: ", command_pool, "\t [CommandPool]");
}


void lvk_destroy(VkDevice device, VkPipelineLayout pipeline_layout) {
	vkDestroyPipelineLayout(device, pipeline_layout, VK_NULL_HANDLE);
	dloggln("Destroyed: ", pipeline_layout, "\t [PipelineLayout]");
}


void lvk_destroy(VkDevice device, VkPipeline pipeline) {
	vkDestroyPipeline(device, pipeline, VK_NULL_HANDLE);
	dloggln("Destroyed: ", pipeline, "\t [Pipeline]");
}


void lvk_destroy(VkDevice device, VkSwapchainKHR swapchain) {
	vkDestroySwapchainKHR(device, swapchain, VK_NULL_HANDLE);
	dloggln("Destroyed: ", swapchain, "\t [SwapchainKHR]");
}


void lvk_destroy(VkDevice device, VkSemaphore semaphore) {
	vkDestroySemaphore(device, semaphore, VK_NULL_HANDLE);
	dloggln("Destroyed: ", semaphore, "\t [Semaphore]");
}


void lvk_destroy(VkDevice device, VkFence fence) {
	vkDestroyFence(device, fence, VK_NULL_HANDLE);
	dloggln("Destroyed: ", fence, "\t [Fence]");
}


void lvk_destroy(VkDevice device, VkDescriptorSetLayout descriptor_set_layout) {
	vkDestroyDescriptorSetLayout(device, descriptor_set_layout, VK_NULL_HANDLE);
	dloggln("Destroyed: ", descriptor_set_layout, "\t [DescriptorSetLayout]");
}


void lvk_destroy(VkDevice device, VkDescriptorPool descriptor_pool) {
	vkDestroyDescriptorPool(device, descriptor_pool, VK_NULL_HANDLE);
	dloggln("Destroyed: ", descriptor_pool, "\t [DescriptorPool]");
}


void lvk_destroy(VkDevice device, VkFramebuffer framebuffer) {
	vkDestroyFramebuffer(device, framebuffer, VK_NULL_HANDLE);
	dloggln("Destroyed: ", framebuffer, "\t [Framebuffer]");
}


void lvk_destroy(VkDevice device, VkRenderPass render_pass) {
	vkDestroyRenderPass(device, render_pass, VK_NULL_HANDLE);
	dloggln("Destroyed: ", render_pass, "\t [RenderPass]");
}


void lvk_destroy(VkDevice device, VkImageView image_view) {
	if (image_view == VK_NULL_HANDLE) { return; }

	vkDestroyImageView(device, image_view, VK_NULL_HANDLE);
	dloggln("Destroyed: ", image_view, "\t [ImageView]");
}


void lvk_destroy(VkDevice device, VkShaderModule shader_module) {
	vkDestroyShaderModule(device, shader_module, VK_NULL_HANDLE);
	dloggln("Destroyed: ", shader_module, "\t [ShaderModule]");
}


void lvk_destroy(VkDevice device, VkSampler sampler) {
	vkDestroySampler(device, sampler, VK_NULL_HANDLE);
	dloggln("Destroyed: ", sampler, "\t [Sampler]");
}


void lvk_destroy(VkDevice device, VkCommandBuffer command_buffer, VkCommandPool command_pool) {
	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
	dloggln("Destroyed: ", command_buffer, "\t [CommandBuffer]");
}

void lvk_destroy(VkDevice device, VkCommandBuffer* command_buffer, uint32_t command_buffer_count, VkCommandPool command_pool) {
	assert(command_buffer_count);

	vkFreeCommandBuffers(device, command_pool, command_buffer_count, command_buffer);
	dloggln("Destroyed: ", command_buffer[0], "\t [CommandBuffer]");
}


void lvk_destroy(VkDevice device, VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool) {
	vkFreeDescriptorSets(device, descriptor_pool, 1, &descriptor_set);
	dloggln("Destroyed: ", descriptor_set, "\t [DescriptionSet]");
}

void lvk_destroy(VkDevice device, VkDescriptorSet* descriptor_set, uint32_t descriptor_set_count, VkDescriptorPool descriptor_pool) {
	assert(descriptor_set_count);

	vkFreeDescriptorSets(device, descriptor_pool, descriptor_set_count, descriptor_set);
	dloggln("Destroyed: ", descriptor_set[0], "\t [DescriptionSet]");
}


void lvk_destroy(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation) {
	vmaDestroyBuffer(allocator, buffer, allocation);
	dloggln("Destroyed: ", buffer, "\t [Buffer]");
}


void lvk_destroy(VmaAllocator allocator, VkImage image, VmaAllocation allocation) {
	vmaDestroyImage(allocator, image, allocation);
	dloggln("Destroyed: ", image, "\t [Image]");
}







