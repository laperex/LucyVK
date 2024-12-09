#include "lucyvk/device.h"
#include "lucyvk/create_info.h"
// #include "lucyvk/types.h"
#include <string>

#include "lucyvk/functions.h"

#include "lucyvk/instance.h"


#include "lucyio/logger.h"
#include <set>
#include <stdexcept>

#define VMA_IMPLEMENTATION

#include "vk_mem_alloc.h"


// |--------------------------------------------------
// ----------------> DEVICE
// |--------------------------------------------------


// lvk_device_ptr lvk_instance::init_device(std::vector<const char*> extensions, std::vector<const char*> layers, lvk::SelectPhysicalDevice_F function) {
// 	return std::make_unique<lvk_device>(this, extensions, layers, function);
// }


// lvk_device::lvk_device(lvk_instance* _instance, std::vector<const char*> extensions, std::vector<const char*> layers, lvk::SelectPhysicalDevice_F function)
// 	: extensions(extensions), layers(layers), instance(_instance)
// {
	
	
// 	// return device;
// }

void lvk_device::wait_idle() const {
	vkDeviceWaitIdle(_device);
}

VkResult lvk_device::submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const VkFence fence, uint64_t timeout) const {
	return vkQueueSubmit(_queue.graphics, 1, submit_info, fence);
}

// VkResult lvk_device::submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const VkFence fence, uint64_t timeout) const {
// 	return vkQueueSubmit(_queue.graphics, 1, submit_info, fence);
// }

VkResult lvk_device::submit2(const VkSubmitInfo2* submit_info2, const uint32_t submit_info2_count, const VkFence fence) const {
	return vkQueueSubmit2(_queue.graphics, submit_info2_count, submit_info2, fence);
}

VkResult lvk_device::present(const VkPresentInfoKHR present_info) const {
	return vkQueuePresentKHR(_queue.present, &present_info);
}

VkResult lvk_device::present(const uint32_t image_index, const VkSwapchainKHR* swapchains, const uint32_t swapchain_count, const VkSemaphore* semaphores, const uint32_t semaphore_count) const {
	return present({
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		
		.waitSemaphoreCount = semaphore_count,
		.pWaitSemaphores = semaphores,
		
		.swapchainCount = swapchain_count,
		.pSwapchains = swapchains,

		.pImageIndices = &image_index,
	});
}

VkResult lvk_device::present(const uint32_t image_index, const VkSwapchainKHR swapchain, const VkSemaphore semaphore) const {
	return present(image_index, &swapchain, 1, &semaphore, 1);
}


void lvk_device::destroy(VkCommandPool command_pool) {
	vkDestroyCommandPool(_device, command_pool, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", command_pool, "\t [CommandPool]");
	destroyer.delete_insert(command_pool);
}


void lvk_device::destroy(VkPipelineLayout pipeline_layout) {
	vkDestroyPipelineLayout(_device, pipeline_layout, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", pipeline_layout, "\t [PipelineLayout]");
	destroyer.delete_insert(pipeline_layout);
}


void lvk_device::destroy(VkPipeline pipeline) {
	vkDestroyPipeline(_device, pipeline, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", pipeline, "\t [Pipeline]");
	destroyer.delete_insert(pipeline);
}


void lvk_device::destroy(VkSwapchainKHR swapchain) {
	vkDestroySwapchainKHR(_device, swapchain, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", swapchain, "\t [SwapchainKHR]");
	destroyer.delete_insert(swapchain);
}


void lvk_device::destroy(VkSemaphore semaphore) {
	vkDestroySemaphore(_device, semaphore, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", semaphore, "\t [Semaphore]");
	destroyer.delete_insert(semaphore);
}


void lvk_device::destroy(VkFence fence) {
	vkDestroyFence(_device, fence, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", fence, "\t [Fence]");
	destroyer.delete_insert(fence);
}


void lvk_device::destroy(VkDescriptorSetLayout descriptor_set_layout) {
	vkDestroyDescriptorSetLayout(_device, descriptor_set_layout, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", descriptor_set_layout, "\t [DescriptorSetLayout]");
	destroyer.delete_insert(descriptor_set_layout);
}


void lvk_device::destroy(VkDescriptorPool descriptor_pool) {
	vkDestroyDescriptorPool(_device, descriptor_pool, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", descriptor_pool, "\t [DescriptorPool]");
	destroyer.delete_insert(descriptor_pool);
}


void lvk_device::destroy(VkFramebuffer framebuffer) {
	dloggln("[ERR] \t", framebuffer, "\t [Framebuffer]");
	vkDestroyFramebuffer(_device, framebuffer, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", framebuffer, "\t [Framebuffer]");
	destroyer.delete_insert(framebuffer);
}


void lvk_device::destroy(VkRenderPass render_pass) {
	vkDestroyRenderPass(_device, render_pass, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", render_pass, "\t [RenderPass]");
	destroyer.delete_insert(render_pass);
}


void lvk_device::destroy(VkImageView image_view) {
	if (image_view == VK_NULL_HANDLE) { return; }

	destroyer.delete_insert(image_view);
	vkDestroyImageView(_device, image_view, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", image_view, "\t [ImageView]");
}


void lvk_device::destroy(VkShaderModule shader_module) {
	vkDestroyShaderModule(_device, shader_module, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", shader_module, "\t [ShaderModule]");
	destroyer.delete_insert(shader_module);
}


void lvk_device::destroy(VkSampler sampler) {
	vkDestroySampler(_device, sampler, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", sampler, "\t [Sampler]");
	destroyer.delete_insert(sampler);
}


// void lvk_device::destroy(VkCommandBuffer command_buffer, VkCommandPool command_pool) {
// 	vkFreeCommandBuffers(_device, command_pool, 1, &command_buffer);
// 	dloggln("DESTROYED \t", command_buffer, "\t [CommandBuffer]");
// }

// 	destroyer.delete_insert(command_buffer);
void lvk_device::destroy(VkCommandBuffer* command_buffer, uint32_t command_buffer_count, VkCommandPool command_pool) {
	assert(command_buffer_count);


	vkFreeCommandBuffers(_device, command_pool, command_buffer_count, command_buffer);
	dloggln("DESTROYED \t", command_buffer[0], "\t [CommandBuffer]");
	destroyer.delete_insert(command_buffer[0]);
}

void lvk_device::destroy(VkBuffer buffer, VmaAllocation allocation) {
	vmaDestroyBuffer(_allocator, buffer, allocation);
	dloggln("DESTROYED \t", buffer, "\t [Buffer]");
	destroyer.delete_insert(buffer);
}

void lvk_device::destroy(const lvk_buffer& buffer) {
	destroy(buffer, buffer._allocation);
}

void lvk_device::destroy(VkImage image, VmaAllocation allocation) {
	vmaDestroyImage(_allocator, image, allocation);
	dloggln("DESTROYED \t", image, "\t [Image]");
	destroyer.delete_insert(image);
}

void lvk_device::destroy(const lvk_image& image) {
	destroy(image, image._allocation);
}


// void lvk_device::destroy(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool) {
// 	vkFreeDescriptorSets(_device, descriptor_pool, 1, &descriptor_set);
// 	dloggln("DESTROYED \t", descriptor_set, "\t [DescriptionSet]");
// }

// void lvk_device::destroy(VkDescriptorSet* descriptor_set, uint32_t descriptor_set_count, VkDescriptorPool descriptor_pool) {
// 	assert(descriptor_set_count);

// 	vkFreeDescriptorSets(_device, descriptor_pool, descriptor_set_count, descriptor_set);
// 	dloggln("DESTROYED \t", descriptor_set[0], "\t [DescriptionSet]");
// }

void lvk_device::destroy() {
	destroyer.flush = true;
	
	int i = 0;
	for (auto* _node = destroyer.delete_queue.end(); _node != nullptr; _node = _node->prev) {
		const auto& element = _node->value;

		if (element.type == typeid(VkCommandPool).hash_code()) {
			destroy(static_cast<VkCommandPool>(element.data[0]));
		}
		if (element.type == typeid(VkPipelineLayout).hash_code()) {
			destroy(static_cast<VkPipelineLayout>(element.data[0]));
		}
		if (element.type == typeid(VkPipeline).hash_code()) {
			destroy(static_cast<VkPipeline>(element.data[0]));
		}
		if (element.type == typeid(VkSwapchainKHR).hash_code()) {
			destroy(static_cast<VkSwapchainKHR>(element.data[0]));
		}
		if (element.type == typeid(VkSemaphore).hash_code()) {
			destroy(static_cast<VkSemaphore>(element.data[0]));
		}
		if (element.type == typeid(VkFence).hash_code()) {
			destroy(static_cast<VkFence>(element.data[0]));
		}
		if (element.type == typeid(VkDescriptorSetLayout).hash_code()) {
			destroy(static_cast<VkDescriptorSetLayout>(element.data[0]));
		}
		if (element.type == typeid(VkDescriptorPool).hash_code()) {
			destroy(static_cast<VkDescriptorPool>(element.data[0]));
		}
		if (element.type == typeid(VkFramebuffer).hash_code()) {
			destroy(static_cast<VkFramebuffer>(element.data[0]));
		}
		if (element.type == typeid(VkRenderPass).hash_code()) {
			destroy(static_cast<VkRenderPass>(element.data[0]));
		}
		if (element.type == typeid(VkImageView).hash_code()) {
			destroy(static_cast<VkImageView>(element.data[0]));
		}
		if (element.type == typeid(VkShaderModule).hash_code()) {
			destroy(static_cast<VkShaderModule>(element.data[0]));
		}
		if (element.type == typeid(VkSampler).hash_code()) {
			destroy(static_cast<VkSampler>(element.data[0]));
		}

		if (element.type == typeid(VkCommandBuffer).hash_code()) {
			destroy((VkCommandBuffer*)element.data.data(), element.data.size() - 1, (VkCommandPool)element.data.back());
		}

		if (element.type == typeid(VkBuffer).hash_code()) {
			destroy((VkBuffer)element.data[0], (VmaAllocation)element.data[1]);
		}
		
		if (element.type == typeid(VkImage).hash_code()) {
			destroy((VkImage)element.data[0], (VmaAllocation)element.data[1]);
		}
	}

	vmaDestroyAllocator(_allocator);
	dloggln("DESTROYED \t", _allocator, "\t [Allocator]");

	vkDestroyDevice(_device, VK_NULL_HANDLE);
	dloggln("DESTROYED \t", _device, "\t [LogicalDevice]");

	dloggln(destroyer.delete_queue.size());
}


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


lvk_semaphore lvk_device::create_semaphore() {
	lvk_semaphore semaphore = {
		._semaphore = VK_NULL_HANDLE
	};
	
	VkSemaphoreCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
	};

	if (vkCreateSemaphore(this->_device, &create_info, VK_NULL_HANDLE, &semaphore._semaphore) != VK_SUCCESS) {
		throw std::runtime_error("semaphore creation failed");
	}
	dloggln("CREATED \t", semaphore._semaphore, "\t [Semaphore]");
	
	destroyer.push(semaphore);

	return semaphore;
}


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


lvk_fence lvk_device::create_fence(VkFenceCreateFlags flags) {
	lvk_fence fence = {
		._fence = VK_NULL_HANDLE
	};
	
	VkFenceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = flags
	};

	if (vkCreateFence(this->_device, &create_info, VK_NULL_HANDLE, &fence._fence) != VK_SUCCESS) {
		throw std::runtime_error("fence creation failed");
	}
	dloggln("CREATED \t", fence._fence, "\t [Fence]");
	
	destroyer.push(fence);

	return fence;
}

void lvk_device::wait_for_fence(const lvk_fence& fence, uint64_t timeout) const {
	wait_for_fences(&fence, 1, timeout);
}

void lvk_device::wait_for_fences(const lvk_fence* fence, uint32_t fence_count, uint64_t timeout) const {
	vkWaitForFences(this->_device, fence_count, &fence->_fence, true, timeout);
}

void lvk_device::reset_fence(const lvk_fence& fence) const {
	reset_fences(&fence, 1);
}

void lvk_device::reset_fences(const lvk_fence* fence, uint32_t fence_count) const {
	vkResetFences(this->_device, fence_count, &fence->_fence);
}



// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


lvk_command_pool lvk_device::create_graphics_command_pool() {
	return create_command_pool(_queue.graphics.index.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

// lvk_immediate_command lvk_device::create_immediate_command() {
// 	lvk_command_pool pool = create_graphics_command_pool();

// 	return {
// 		.command_pool = pool,
// 		.command_buffer = allocate_command_buffer_unique(pool),
// 		.fence = create_fence()
// 	};
// }


lvk_command_pool lvk_device::create_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
	lvk_command_pool command_pool = {
		._command_pool = VK_NULL_HANDLE
	};

	VkCommandPoolCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = flags,
		.queueFamilyIndex = queue_family_index,
	};

    if (vkCreateCommandPool(_device, &create_info, VK_NULL_HANDLE, &command_pool._command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
	dloggln("CREATED \t", command_pool._command_pool, "\t [CommandPool]");
	
	destroyer.push(command_pool);

	return command_pool;
}

void lvk_device::reset_command_pool(const lvk_command_pool& command_pool) {
	vkResetCommandPool(_device, command_pool._command_pool, 0);
}

lvk_command_buffer lvk_device::create_command_buffer(const VkCommandPool command_pool, VkCommandBufferLevel level) {
	lvk_command_buffer command_buffer = {
		._command_buffer = VK_NULL_HANDLE
	};

	create_command_buffer_array(&command_buffer, command_pool, 1, level);

	return command_buffer;
}

void lvk_device::create_command_buffer_array(const lvk_command_buffer* command_buffer_array, const VkCommandPool command_pool, uint32_t command_buffer_count, VkCommandBufferLevel level) {
	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

		.commandPool = command_pool,
		.level = level,
		.commandBufferCount = command_buffer_count,
	};

	if (vkAllocateCommandBuffers(this->_device, &allocate_info, (VkCommandBuffer*)command_buffer_array) != VK_SUCCESS) {
		throw std::runtime_error("command buffers allocation failed!");
	}
	dloggln("ALLOCATED \t", command_buffer_array[0]._command_buffer, "\t [CommandBufferArray]");
	
	destroyer.push((VkCommandBuffer*)command_buffer_array, command_buffer_count, command_pool);
	
	// return command_buffer_array;
}


VkResult lvk_device::imm_submit(std::function<void(lvk_command_buffer)> function) {
	static struct {
		VkCommandPool _command_pool = VK_NULL_HANDLE;
		VkCommandBuffer _command_buffer = VK_NULL_HANDLE;
		VkFence _fence = VK_NULL_HANDLE;
	} immediate_command;

	if (immediate_command._fence == VK_NULL_HANDLE) {
		immediate_command._fence = create_fence();
		immediate_command._command_pool = create_command_pool(_queue.graphics.index.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
		immediate_command._command_buffer = create_command_buffer(immediate_command._command_pool);

		dloggln("CREATED \t", "[Immediate Commands]");
	}

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = VK_NULL_HANDLE
	};
	
	auto imm_cmd = static_cast<lvk_command_buffer>(immediate_command._command_buffer);

	vkBeginCommandBuffer(immediate_command._command_buffer, &begin_info);
	
	function(imm_cmd);
	
	vkEndCommandBuffer(immediate_command._command_buffer);
	
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		
		.commandBufferCount = 1,
		.pCommandBuffers = &immediate_command._command_buffer,
	};
	
	VkResult result = vkQueueSubmit(_queue.graphics, 1, &submit_info, immediate_command._fence);
	
	vkWaitForFences(_device, 1, &immediate_command._fence, true, LVK_TIMEOUT);
	vkResetFences(_device, 1, &immediate_command._fence);
	
	vkResetCommandPool(_device, immediate_command._command_pool, 0);
	
	return result;
}

// VkResult lvk_device::imm_buffer_copy(const VkBuffer src_buffer, const VkBuffer dst_buffer, const VkDeviceSize size) {
// 	return this->imm_submit([&](VkCommandBuffer cmd) {
// 		lvk_command_buffer command_buffer = static_cast<lvk_command_buffer>(cmd);
		
// 		command_buffer.copy_buffer_to_buffer(src_buffer, dst_buffer, {
// 			{
// 				.srcOffset = 0,
// 				.dstOffset = 0,
// 				.size = size
// 			}
// 		});
// 	});
// }

// VkResult lvk_device::imm_buffer_upload(const lvk_buffer& buffer) {
	
// }


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


lvk_swapchain lvk_device::create_swapchain(VkRenderPass render_pass, uint32_t width, uint32_t height, VkImageUsageFlags image_usage_flags, VkSurfaceFormatKHR surface_format) {
	const auto& capabilities = _swapchain_support_details.capabilities;
	// _swapchain_support_details.capabilities;

	lvk_swapchain swapchain = {
		._swapchain = VK_NULL_HANDLE,
		._extent = VkExtent2D { width, height },
		._surface_format = surface_format,
		._present_mode = VK_PRESENT_MODE_FIFO_KHR,
		._image_usage = image_usage_flags,
		// .device = this,
		// .physical_device = physical_device,
		// .instance = instance
	};

	// TODO: support for more presentMode types
	// * VK_PRESENT_MODE_IMMEDIATE_KHR = Not suggested causes tearing
	// * VK_PRESENT_MODE_MAILBOX_KHR = Mailbox
	// * VK_PRESENT_MODE_FIFO_KHR = V-sync
	// * VK_PRESENT_MODE_FIFO_RELAXED_KHR
	// * VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR
	// * VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR

	for (const auto& availablePresentMode: _swapchain_support_details.present_modes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			swapchain._present_mode = availablePresentMode;
			break;
		}
		if (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
			swapchain._present_mode = availablePresentMode;
			break;
		}
	}
	
	dloggln("INFO:\t", lvk::to_string(swapchain._present_mode), "\t [Swapchain PresentMode]");

	swapchain_recreate(swapchain, render_pass, width, height);

	return swapchain;
}


void lvk_device::swapchain_recreate(lvk_swapchain& swapchain, const VkRenderPass render_pass, uint32_t width, uint32_t height) {
	if (swapchain._image_count) {
		swapchain_destroy(swapchain);
	}

	// if (swapchain._swapchain != VK_NULL_HANDLE) {
	// 	vkDestroySwapchainKHR(_device, swapchain._swapchain, VK_NULL_HANDLE);
	// 	dloggln("Swapchain Destroyed");
	// }

	swapchain._extent.width = width;
	swapchain._extent.height = height;
	
	const auto& present_modes = this->_swapchain_support_details.present_modes;
	const auto& capabilities = this->_swapchain_support_details.capabilities;
	
	VkSwapchainPresentScalingCreateInfoEXT present_scaling {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_SCALING_CREATE_INFO_EXT,
		
		.pNext = VK_NULL_HANDLE,
		
		.scalingBehavior = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT,
		.presentGravityX = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT,
		.presentGravityY = VK_PRESENT_SCALING_ONE_TO_ONE_BIT_EXT,
	};
	
	VkSwapchainCreateInfoKHR create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = &present_scaling,
		.flags = 0,
		.surface = _surfaceKHR,

		// TF ???
		.minImageCount = (capabilities.maxImageCount > 0 && capabilities.minImageCount + 1 > capabilities.maxImageCount) ? capabilities.maxImageCount: capabilities.minImageCount + 1,

		.imageFormat = swapchain._surface_format.format,
		.imageColorSpace = swapchain._surface_format.colorSpace,
		.imageExtent = swapchain._extent,
			// (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) ?
			// capabilities.currentExtent:
			// VkExtent2D {
			// 	std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, swapchain._extent.width)),
			// 	std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, swapchain._extent.height))
			// },
		.imageArrayLayers = 1,
		.imageUsage = swapchain._image_usage,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = swapchain._present_mode,
		
		// TODO: user defined clipping state
		.clipped = VK_TRUE,
		
		// TODO: remains to be tested
		.oldSwapchain = swapchain,
	};
	
	// TODO: better approach
	uint32_t queue_family_indices[] = {
		this->_queue.graphics.index.value(),
		this->_queue.present.index.value()
	};

	// TODO: Sharing Mode is always exclusive in lvk_buffer. Therefore only one queue is possible
	if (this->_queue.present == this->_queue.graphics) {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = nullptr;
	} else {
		throw std::runtime_error("VK_SHARING_MODE_CONCURRENT is not implemented yet");

		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = std::size(queue_family_indices);
		create_info.pQueueFamilyIndices = queue_family_indices;
	}
	
	auto result = vkCreateSwapchainKHR(this->_device, &create_info, VK_NULL_HANDLE, &swapchain._swapchain);
	
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create swapchain!" + std::to_string((int)result));
	}
	dloggln("CREATED \t", swapchain, "\t [Swapchain]");

	// dloggln("INFO \t", "[", create_info.imageExtent.width, ", ", create_info.imageExtent.height, "]");
	// dloggln("INFO swapchain._extent \t", "[", swapchain._extent.width, ", ", swapchain._extent.height, "]");
	
	// Depth & ImageViews & Framebuffers

	if (swapchain._image_views == VK_NULL_HANDLE) {
		swapchain._image_views = new VkImageView[swapchain._image_count];
		swapchain._framebuffers = new VkFramebuffer[swapchain._image_count];
	}

	vkGetSwapchainImagesKHR(this->_device, swapchain._swapchain, &swapchain._image_count, VK_NULL_HANDLE);
	VkImage* _images = new VkImage[swapchain._image_count];
	vkGetSwapchainImagesKHR(this->_device, swapchain._swapchain, &swapchain._image_count, _images);

	swapchain._depth_image = create_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
	swapchain._depth_image_view = create_image_view(swapchain._depth_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);

	// destroyer.push(swapchain);

	for (size_t i = 0; i < swapchain._image_count; i++) {
		swapchain._image_views[i] = create_image_view(_images[i], swapchain._surface_format.format, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
		swapchain._framebuffers[i] = create_framebuffer(render_pass, swapchain._extent, {
			swapchain._image_views[i],
			swapchain._depth_image_view
		});
	}
	dloggln("CREATED \t", "[Swapchain ImageViews]");

	delete [] _images;
}

void lvk_device::swapchain_destroy(lvk_swapchain& swapchain) {
	destroy(swapchain);
	destroy(swapchain._depth_image_view);
	
	swapchain._swapchain = VK_NULL_HANDLE;

	for (int i = 0; i < swapchain._image_count; i++) {
		destroy(swapchain._image_views[i]);
		destroy(swapchain._framebuffers[i]);
	}

	destroy(swapchain._depth_image, swapchain._depth_image._allocation);
}

VkResult lvk_device::swapchain_acquire_next_image(const lvk_swapchain& swapchain, uint32_t* index, VkSemaphore semaphore, VkFence fence, const uint64_t timeout) const {
	return vkAcquireNextImageKHR(this->_device, swapchain._swapchain, timeout, semaphore, fence, index);
}


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


lvk_pipeline_layout lvk_device::create_pipeline_layout(const VkPushConstantRange* push_constant_ranges, const uint32_t push_constant_range_count, const VkDescriptorSetLayout* descriptor_set_layouts, const uint32_t descriptor_set_layout_count) {
	lvk_pipeline_layout pipeline_layout = {
		._pipeline_layout = VK_NULL_HANDLE,
	};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,

		.setLayoutCount = descriptor_set_layout_count,
		.pSetLayouts = descriptor_set_layouts,

		.pushConstantRangeCount = push_constant_range_count,
		.pPushConstantRanges = push_constant_ranges
	};
	
	if (vkCreatePipelineLayout(this->_device, &pipeline_layout_create_info, VK_NULL_HANDLE, &pipeline_layout._pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("pipeline layout creation failed!");
	}
	dloggln("CREATED \t", pipeline_layout._pipeline_layout, "\t [Pipeline Layout]");
	
	destroyer.push(pipeline_layout);
	
	return pipeline_layout;
}


// ! |--------------------------------------------------
// ! ----------------> SHADER
// ! |--------------------------------------------------


lvk_shader_module lvk_device::create_shader_module(const char* filename) {
	lvk_shader_module shader_module = {
		._shader_module = VK_NULL_HANDLE,
		// ._stage = stage
	};
	
	VkShaderModuleCreateInfo info = lvk::info::shader_module(filename);

	if (!info.codeSize) {
		throw std::runtime_error(std::string("shader is invalid") + filename);
	}

	if (vkCreateShaderModule(_device, &info, VK_NULL_HANDLE, &shader_module._shader_module) != VK_SUCCESS) {
		throw std::runtime_error(std::string("failed to create shader module! -> ") + filename);
	}
	dloggln("CREATED \t", shader_module._shader_module, "\t [Shader Module] - ", filename);

	destroyer.push(shader_module);

	return shader_module;
}


// |--------------------------------------------------
// ----------------> PIPELINE
// |--------------------------------------------------


lvk_pipeline lvk_device::create_graphics_pipeline(const VkPipelineLayout pipeline_layout, const lvk::config::graphics_pipeline& config, const VkRenderPass render_pass) {
	lvk_pipeline pipeline = {
		._pipeline = VK_NULL_HANDLE
	};
	
	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,

		.pNext = (config.rendering_info.sType == VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO) ? &config.rendering_info: VK_NULL_HANDLE,

		.stageCount = static_cast<uint32_t>(config.shader_stage_array.size()),
		.pStages = config.shader_stage_array.data(),

		.pVertexInputState = &config.vertex_input_state,
		.pInputAssemblyState = &config.input_assembly_state,
		.pViewportState = &config.viewport_state,
		.pRasterizationState = &config.rasterization_state,
		.pMultisampleState = &config.multisample_state,
		.pDepthStencilState = &config.depth_stencil_state,
		.pColorBlendState = &config.color_blend_state,
		.pDynamicState = &config.dynamic_state,
		.layout = pipeline_layout,
		.renderPass = render_pass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
	};

	if (vkCreateGraphicsPipelines(this->_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("graphics pipeline creation failed!");
	}
	dloggln("CREATED \t", pipeline._pipeline, "\t [Graphics Pipeline]");

	destroyer.push(pipeline);
	
	return pipeline;
}

void lvk_device::create_graphics_pipeline_array(const VkPipeline* pipeline_array, const VkPipelineLayout pipeline_layout, VkGraphicsPipelineCreateInfo* graphics_pipeline_create_info_array, uint32_t graphics_pipeline_create_info_array_size) {
	if (vkCreateGraphicsPipelines(this->_device, VK_NULL_HANDLE, graphics_pipeline_create_info_array_size, graphics_pipeline_create_info_array, nullptr, (VkPipeline*)pipeline_array) != VK_SUCCESS) {
		throw std::runtime_error("graphics pipeline creation failed!");
	}
	dloggln("CREATED \t", (VkPipeline*)pipeline_array, "\t [Graphics Pipeline]");

	for (int i = 0; i < graphics_pipeline_create_info_array_size; i++) {
		destroyer.push(pipeline_array[i]);
	}
}

lvk_pipeline lvk_device::create_graphics_pipeline(const VkPipelineLayout pipeline_layout, VkGraphicsPipelineCreateInfo graphics_pipeline_create_info) {
	lvk_pipeline pipeline = {
		._pipeline = VK_NULL_HANDLE,
	};

	// create_graphics_pipeline()

	return pipeline;
}

lvk_pipeline lvk_device::create_compute_pipeline(const VkPipelineLayout pipeline_layout, const VkPipelineShaderStageCreateInfo stage_info) {
	lvk_pipeline pipeline = {
		._pipeline = VK_NULL_HANDLE,
	};

	VkComputePipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = stage_info,
		.layout = pipeline_layout,
	};

	if (vkCreateComputePipelines(this->_device, VK_NULL_HANDLE, 1, &pipeline_info, VK_NULL_HANDLE, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("compute pipeline creation failed!");
	}
	dloggln("CREATED \t", pipeline._pipeline, "\t [Compute Pipeline]");
	
	destroyer.push(pipeline);

	return pipeline;
}


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


void lvk_device::upload(const VmaAllocation allocation, const VkDeviceSize size, const void* data = nullptr) const {
	void* _data = nullptr;
	vmaMapMemory(this->_allocator, allocation, &_data);
	
	memcpy(_data, data, size);
	
	vmaUnmapMemory(this->_allocator, allocation);
}


lvk_buffer lvk_device::create_buffer(const VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, const VkDeviceSize size, const void* data) {
	lvk_buffer buffer = {
		._buffer = VK_NULL_HANDLE,
		._allocation = VK_NULL_HANDLE,
		._allocated_size = size,
		._usage = buffer_usage,
		._memory_usage = memory_usage,
		// ._allocator = this
		// ._is_static = memory_usage == VMA_MEMORY_USAGE_GPU_ONLY ? VK_TRUE: VK_FALSE
	};
	
	VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = buffer_usage,
		
		// TODO: Logical Device Implementation does not support seperate presentation and graphics queue (VK_SHARING_MODE_CONCURRENT) yet
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,

		// .queueFamilyIndexCount = queue_family_indices_count,
		// .pQueueFamilyIndices = queue_family_indices
	};

	VmaAllocationCreateInfo vmaallocInfo = {
		.usage = memory_usage
	};

	//allocate the buffer
	if (vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &buffer._buffer, &buffer._allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	dloggln("CREATED \t", buffer._buffer, "\t [Buffer]");
	
	if (data != nullptr) {
		upload(buffer, size, data);
	}

	destroyer.push(buffer, buffer._allocation);

	return buffer;
}

lvk_buffer lvk_device::create_staging_buffer(const VkDeviceSize size, const void* data) {
	return create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, size, data);
}

lvk_buffer lvk_device::create_index_buffer(const VkDeviceSize size, const void* data) {
	return create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, size, data);
}

lvk_buffer lvk_device::create_index_buffer_static(const VkDeviceSize size, const void* data) {
	return create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, size, data);
}

lvk_buffer lvk_device::create_vertex_buffer(const VkDeviceSize size, const void* data) {
	return create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, size, data);
}

lvk_buffer lvk_device::create_vertex_buffer_static(const VkDeviceSize size, const void* data) {
	return create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, size, data);
}

lvk_buffer lvk_device::create_uniform_buffer(const VkDeviceSize size, const void* data) {
	return create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, size, data);
}

lvk_buffer lvk_device::create_uniform_buffer_static(const VkDeviceSize size, const void* data) {
	return create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, size, data);
}

void lvk_device::upload(const lvk_buffer& buffer, const VkDeviceSize size, const void* data, lvk_buffer staging_buffer) {
	if (size > buffer._allocated_size) {
		throw std::runtime_error("ERROR: required size is greater than allocated size!");
	}

	if (buffer._memory_usage == VMA_MEMORY_USAGE_GPU_ONLY) {
		// throw std::runtime_error("ERROR: unable to directly write to a static buffer!");

		// if (staging_buffer._buffer == VK_NULL_HANDLE) {
			staging_buffer = this->create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, size, data);
		// } else {
		// 	if (staging_buffer._allocated_size < buffer._allocated_size) {
		// 		throw std::runtime_error("ERROR: staging buffer allocated size does not match the data size!");
		// 	}
		// }

		// lvk_destroy(_allocator, staging_buffer._buffer, staging_buffer._allocation);

		this->imm_submit([&](VkCommandBuffer cmd) {
			static_cast<lvk_command_buffer>(cmd).copy_buffer_to_buffer(staging_buffer, buffer, {
				{
					.srcOffset = 0,
					.dstOffset = 0,
					.size = size
				}
			});
		});
		
		destroy(staging_buffer._buffer, staging_buffer._allocation);
	} else {
		upload(buffer._allocation, size, data);
	}
}


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------


lvk_image lvk_device::create_image(VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memory_usage, VkExtent3D extent, VkImageType image_type) {
	lvk_image image = {
		._image = VK_NULL_HANDLE,
		._allocation = VK_NULL_HANDLE,

		._format = format,
		._image_type = image_type,
		._extent = extent,
		._usage = usage,
		._memory_usage = memory_usage,
	};

	VkImageCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,

		.imageType = image_type,
		.format = format,
		.extent = extent,
		
		// TODO: Mipmapping
		.mipLevels = 1,
		
		// TODO: Cubemaps
		.arrayLayers = 1,
		
		// TODO: MSAA
		.samples = VK_SAMPLE_COUNT_1_BIT,

		// * VK_IMAGE_TILING_OPTIMAL					-> Let Vulkan Choose
		// * VK_IMAGE_TILING_LINEAR						-> To read from CPU
		// * VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT	-> Model Specific
		.tiling = VK_IMAGE_TILING_OPTIMAL,

		.usage = usage,
	};

	VmaAllocationCreateInfo allocation_info = {
		.usage = memory_usage,
		.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	};

	if (vmaCreateImage(_allocator, &create_info, &allocation_info, &image._image, &image._allocation, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("image creation failed!");
	}
	dloggln("CREATED \t", image._image, "\t [Image]");

	destroyer.push(image, image._allocation);

	return image;
}




// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


lvk_image_view lvk_device::create_image_view(const VkImage image, VkFormat format, VkImageViewType image_view_type, VkImageAspectFlags aspect_flag) {
	lvk_image_view image_view = {
		._image_view = VK_NULL_HANDLE,
	};

	VkImageViewCreateInfo create_info = lvk::info::image_view(image, format, aspect_flag, image_view_type);

	if (vkCreateImageView(this->_device, &create_info, VK_NULL_HANDLE, &image_view._image_view) != VK_SUCCESS) {
		throw std::runtime_error("image_view creation failed!");
	}
	dloggln("CREATED \t", image_view._image_view, "\t [ImageView]");

	destroyer.push(image_view);

	return image_view;
}

lvk_image_view lvk_device::create_image_view(const lvk_image& image, VkImageViewType image_view_type, VkImageAspectFlags aspect_flag) {
	return create_image_view(image._image, image._format, image_view_type, aspect_flag);
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET LAYOUT
// |--------------------------------------------------


lvk_descriptor_set_layout lvk_device::create_descriptor_set_layout(const VkDescriptorSetLayoutBinding* bindings, const uint32_t binding_count) {
	lvk_descriptor_set_layout descriptor_set_layout = {
		._descriptor_set_layout = VK_NULL_HANDLE
	};

	VkDescriptorSetLayoutCreateInfo set_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = binding_count,
		.pBindings = bindings,
	};

	if (vkCreateDescriptorSetLayout(_device, &set_info, VK_NULL_HANDLE, &descriptor_set_layout._descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("descriptor_set_layout creation failed!");
	}
	dloggln("CREATED \t", descriptor_set_layout._descriptor_set_layout, "\t [DescriptorSetLayout]");

	destroyer.push(descriptor_set_layout);

	return descriptor_set_layout;
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


void lvk_device::create_descriptor_set_array(const lvk_descriptor_set* descriptor_set_array, const VkDescriptorPool descriptor_pool, const VkDescriptorSetLayout* descriptor_set_layout_array, uint32_t descriptor_set_layout_array_size) {
	VkDescriptorSetAllocateInfo allocate_info ={
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,

		.descriptorPool = descriptor_pool,
		.descriptorSetCount = descriptor_set_layout_array_size,
		.pSetLayouts = descriptor_set_layout_array,
	};

	if (vkAllocateDescriptorSets(this->_device, &allocate_info, (VkDescriptorSet*)descriptor_set_array) != VK_SUCCESS) {
		throw std::runtime_error("descriptor_set allocation failed!");
	}
	dloggln("ALLOCATED \t", descriptor_set_array->_descriptor_set, "\t [Description Set]");
}

lvk_descriptor_set lvk_device::create_descriptor_set(const VkDescriptorPool descriptor_pool, const VkDescriptorSetLayout descriptor_set_layout) {
	lvk_descriptor_set descriptor_set;

	create_descriptor_set_array(&descriptor_set, descriptor_pool, &descriptor_set_layout, 1);

	return descriptor_set;
}

void lvk_device::update_descriptor_set(const lvk_descriptor_set& descriptor_set, uint32_t binding, const lvk_buffer* buffer, VkDescriptorType type, const std::size_t offset) const {
	VkDescriptorBufferInfo buffer_info = {
		.buffer = buffer->_buffer,
		.offset = offset,
		.range = buffer->_allocated_size
	};
	
	VkWriteDescriptorSet write_set = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptor_set._descriptor_set,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = type,
		.pBufferInfo = &buffer_info,
	};

	vkUpdateDescriptorSets(this->_device, 1, &write_set, 0, VK_NULL_HANDLE);
}

void lvk_device::update_descriptor_set(const lvk_descriptor_set& descriptor_set, uint32_t binding, const lvk_image_view* image_view, const lvk_sampler& sampler, VkImageLayout image_layout, VkDescriptorType type, const std::size_t offset) const {
	VkDescriptorImageInfo image_info = {
		.sampler = sampler._sampler,
		.imageView = image_view->_image_view,
		.imageLayout = image_layout,
	};
	
	VkWriteDescriptorSet write_set = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptor_set._descriptor_set,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = type,
		.pImageInfo = &image_info,
	};

	vkUpdateDescriptorSets(this->_device, 1, &write_set, 0, VK_NULL_HANDLE);
}



// |--------------------------------------------------
// ----------------> DESCRIPTOR POOL
// |--------------------------------------------------


lvk_descriptor_pool lvk_device::create_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize* descriptor_pool_sizes, const uint32_t descriptor_pool_sizes_count) {
	lvk_descriptor_pool descriptor_pool = {
		._descriptor_pool = VK_NULL_HANDLE,
	};

	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = max_descriptor_sets,
		.poolSizeCount = descriptor_pool_sizes_count,
		.pPoolSizes = descriptor_pool_sizes
	};

	if (vkCreateDescriptorPool(_device, &pool_info, VK_NULL_HANDLE, &descriptor_pool._descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("descriptor_pool creation failed");
	}
	dloggln("CREATED \t", descriptor_pool, "\t [DescriptorPool]");

	destroyer.push(descriptor_pool);

	return descriptor_pool;
}

void lvk_device::clear_descriptor_pool(const lvk_descriptor_pool& descriptor_pool) const {
	vkResetDescriptorPool(this->_device, descriptor_pool._descriptor_pool, 0);
}



// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


lvk_render_pass lvk_device::create_default_render_pass(VkFormat format) {	
	VkAttachmentDescription attachments[2] = {
		{
			.format = format,

			// TODO: MSAA
			.samples = VK_SAMPLE_COUNT_1_BIT,

			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,

			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		},
		{
			.format = VK_FORMAT_D32_SFLOAT,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		}
	};

	VkSubpassDependency dependency[2] = {
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,

			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,

			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		},
		{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			
			.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,

			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		}
	};

	VkAttachmentReference attachment_ref[2] = {
		{
			.attachment = 0,
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		},
		{
			.attachment = 1,
			.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		}
	};
	
	// lvk::info::subp

	VkSubpassDescription subpasses[1] = {
		{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = 1,

			.pColorAttachments = &attachment_ref[0],
			.pDepthStencilAttachment = &attachment_ref[1],
		}
	};

	return create_render_pass(attachments, subpasses, dependency, false);
}

lvk_render_pass lvk_device::create_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform) {
	lvk_render_pass render_pass = {
		._render_pass = VK_NULL_HANDLE,
	};

	VkRenderPassCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,

		.flags = static_cast<VkRenderPassCreateFlags>((enable_transform == true) ? VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM: 0),

		.attachmentCount = attachment_count,
		.pAttachments = attachment,
		.subpassCount = subpass_count,
		.pSubpasses = subpass,
		.dependencyCount = dependency_count,
		.pDependencies = dependency,
	};

	if (vkCreateRenderPass(_device, &create_info, VK_NULL_HANDLE, &render_pass._render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create renderpass!");
	}
	dloggln("CREATED \t", render_pass, "\t [RenderPass]");
	
	destroyer.push(render_pass);

	return render_pass;
}


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


lvk_framebuffer lvk_device::create_framebuffer(const VkRenderPass render_pass, const VkExtent2D extent, const VkImageView* image_views, const uint32_t image_views_count) {
	lvk_framebuffer framebuffer = {
		._framebuffer = VK_NULL_HANDLE,
		._extent = extent
	};
	
	VkFramebufferCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		
		// .flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
		
		.renderPass = render_pass,
		
		.attachmentCount = image_views_count,
		.pAttachments = image_views,
		
		.width = extent.width,
		.height = extent.height,
		
		.layers = 1,
	};
	
	if (vkCreateFramebuffer(this->_device, &create_info, VK_NULL_HANDLE, &framebuffer._framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer");
	}
	dloggln("CREATED \t", framebuffer, "\t [Framebuffer]");
	
	destroyer.push(framebuffer);

	return framebuffer;
}


// |--------------------------------------------------
// ----------------> SAMPLER
// |--------------------------------------------------


lvk_sampler lvk_device::create_sampler(VkFilter min_filter, VkFilter mag_filter, VkSamplerAddressMode sampler_addres_mode) {
	lvk_sampler sampler {
		._sampler = VK_NULL_HANDLE
	};

	VkSamplerCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		
		.magFilter = mag_filter,
		.minFilter = min_filter,
		
		.addressModeU = sampler_addres_mode,
		.addressModeV = sampler_addres_mode,
		.addressModeW = sampler_addres_mode,
	};
	
	if (vkCreateSampler(this->_device, &create_info, VK_NULL_HANDLE, &sampler._sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer");
	}
	dloggln("CREATED \t", sampler, "\t [Sampler]");
	
	destroyer.push(sampler);

	return sampler;
}

lvk_image lvk_device::load_image(VkDeviceSize size, void* data, VkExtent3D extent, VkImageType type, VkFormat format) {
	lvk_buffer staging_buffer = create_staging_buffer(size, data);

	lvk_image image = create_image(format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, extent, VK_IMAGE_TYPE_2D);

	imm_submit([&](lvk_command_buffer command_buffer) {
		VkImageMemoryBarrier image_barrier = lvk::info::image_memory_barrier(image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, lvk::info::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT));

		command_buffer.pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, image_barrier);
		command_buffer.copy_buffer_to_image(staging_buffer, image, image._extent);

		image_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		command_buffer.pipeline_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, image_barrier);
	});

	destroy(staging_buffer);

	return image;
}