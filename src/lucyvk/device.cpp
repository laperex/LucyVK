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

void lvk_device::destroy_device() {
	deletion_queue.flush();

	vmaDestroyAllocator(allocator);
	dloggln("Allocator Destroyed");

	vkDestroyDevice(_device, VK_NULL_HANDLE);
	dloggln("Logical Device Destroyed");
}

// lvk_device::~lvk_device() {
// 	dloggln("-- Device Destructor");
// }

// void lvk_device::allocate_command_buffers(const VkCommandPool command_pool, VkCommandBufferLevel level, lvk_command_buffer* command_buffers, uint32_t command_buffers_count) {
// 	// lvk_command_buffer command_buffer = {
// 	// 	._command_buffer = VK_NULL_HANDLE,
// 	// };

// 	VkCommandBufferAllocateInfo allocate_info = {
// 		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

// 		.commandPool = command_pool,
// 		.level = level,
// 		.commandBufferCount = 1,
// 	};

// 	if (vkAllocateCommandBuffers(device->_device, &allocate_info, &command_buffer._command_buffer) != VK_SUCCESS) {
// 		throw std::runtime_error("command buffer allocation failed!");
// 	}
// 	dloggln("Command Buffer Allocated: ", &command_buffer._command_buffer);
// }



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
	dloggln("Semaphore Created");
	
	deletion_queue.push([=]{
		vkDestroySemaphore(this->_device, semaphore._semaphore, VK_NULL_HANDLE);
		dloggln("Semaphore Destroyed");
	});
	
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
	dloggln("Fence Created");
	
	deletion_queue.push([=]{
		vkDestroyFence(_device, fence._fence, VK_NULL_HANDLE);
		dloggln("Fence Destroyed");
	});
	
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
	
	deletion_queue.push([=]{
		vkDestroyCommandPool(_device, command_pool._command_pool, VK_NULL_HANDLE);
		dloggln("Command Pool Destroyed");
	});

	return command_pool;
}

void lvk_device::reset_command_pool(const lvk_command_pool& command_pool) {
	vkResetCommandPool(_device, command_pool._command_pool, 0);
}

lvk_command_buffer lvk_device::create_command_buffer_unique(const lvk_command_pool& command_pool, VkCommandBufferLevel level) {
	lvk_command_buffer command_buffer = {
		._command_buffer = VK_NULL_HANDLE
	};

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

		.commandPool = command_pool._command_pool,
		.level = level,
		.commandBufferCount = 1,
	};

	if (vkAllocateCommandBuffers(this->_device, &allocate_info, &command_buffer._command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("command buffer allocation failed!");
	}
	dloggln("Command Buffer Allocated: ");
	
	
	deletion_queue.push([=]{
		vkFreeCommandBuffers(this->_device, command_pool._command_pool, 1, &command_buffer._command_buffer);
		dloggln("Command Buffer Destroyed");
	});
	
	return command_buffer;
}

std::vector<lvk_command_buffer> lvk_device::create_command_buffers(const lvk_command_pool& command_pool, uint32_t command_buffer_count, VkCommandBufferLevel level) {
	const std::vector<lvk_command_buffer> command_buffer_array(command_buffer_count);

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

		.commandPool = command_pool._command_pool,
		.level = level,
		.commandBufferCount = command_buffer_count,
	};

	if (vkAllocateCommandBuffers(this->_device, &allocate_info, (VkCommandBuffer*)command_buffer_array.data()->_command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("command buffers allocation failed!");
	}
	dloggln("Command Buffers Allocated: ");
	
	deletion_queue.push([=]{
		vkFreeCommandBuffers(this->_device, command_pool._command_pool, command_buffer_count, (VkCommandBuffer*)command_buffer_array.data()->_command_buffer);
		dloggln("Command Buffers Destroyed");
	});
	
	return command_buffer_array;
}


VkResult lvk_device::imm_submit(std::function<void(const VkCommandBuffer)> function) {
	static struct {
		VkCommandPool _command_pool = VK_NULL_HANDLE;
		VkCommandBuffer _command_buffer = VK_NULL_HANDLE;
		VkFence _fence = VK_NULL_HANDLE;
	} immediate_command;


	if (immediate_command._command_pool == VK_NULL_HANDLE) {
		VkCommandPoolCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = _queue.graphics.index.value(),
		};

		if (vkCreateCommandPool(_device, &create_info, VK_NULL_HANDLE, &immediate_command._command_pool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}
	
	if (immediate_command._command_buffer == VK_NULL_HANDLE) {
		VkCommandBufferAllocateInfo allocate_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

			.commandPool = immediate_command._command_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		if (vkAllocateCommandBuffers(this->_device, &allocate_info, &immediate_command._command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("command buffers allocation failed!");
		}
	}

	if (immediate_command._fence == VK_NULL_HANDLE) {
		VkFenceCreateInfo create_info = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = VK_NULL_HANDLE,
			.flags = 0
		};

		if (vkCreateFence(this->_device, &create_info, VK_NULL_HANDLE, &immediate_command._fence) != VK_SUCCESS) {
			throw std::runtime_error("fence creation failed");
		}
		
		deletion_queue.push([=, this]{
			vkFreeCommandBuffers(_device, immediate_command._command_pool, 1, &immediate_command._command_buffer);
			vkDestroyCommandPool(_device, immediate_command._command_pool, VK_NULL_HANDLE);
			vkDestroyFence(_device, immediate_command._fence, VK_NULL_HANDLE);
			dloggln("Immediate Commands Destroyed");
		});
	}

	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = VK_NULL_HANDLE
	};

	vkBeginCommandBuffer(immediate_command._command_buffer, &begin_info);
	
	function(immediate_command._command_buffer);
	
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


lvk_swapchain lvk_device::create_swapchain(uint32_t width, uint32_t height, VkImageUsageFlags image_usage_flags, VkSurfaceFormatKHR surface_format) {
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

	swapchain_recreate(swapchain, width, height);
	
	deletion_queue.push([=]{
		vkDestroySwapchainKHR(_device, swapchain._swapchain, VK_NULL_HANDLE);
		dloggln("Swapchain Destroyed");

		for (int i = 0; i < swapchain._image_count; i++) {
			vkDestroyImageView(_device, swapchain._image_views[i], VK_NULL_HANDLE);
		}
		dloggln("ImageViews Destroyed");
		
		delete [] swapchain._image_views;
		delete [] swapchain._images;
	});

	return swapchain;
}


void lvk_device::swapchain_recreate(lvk_swapchain& swapchain, uint32_t width, uint32_t height) const {
	if (swapchain._image_count) {
		for (int i = 0; i < swapchain._image_count; i++) {
			vkDestroyImageView(this->_device, swapchain._image_views[i], VK_NULL_HANDLE);
		}
		dloggln("ImageViews Destroyed");
	}

	if (swapchain._swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(this->_device, swapchain._swapchain, VK_NULL_HANDLE);
		dloggln("Swapchain Destroyed");
	}
	
	swapchain._extent.width = width;
	swapchain._extent.height = height;
	
	const auto& present_modes = this->_swapchain_support_details.present_modes;
	const auto& capabilities = this->_swapchain_support_details.capabilities;
	
	VkSwapchainCreateInfoKHR create_info = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = _surfaceKHR,

		// TF ???
		.minImageCount = (capabilities.maxImageCount > 0 && capabilities.minImageCount + 1 > capabilities.maxImageCount) ? capabilities.maxImageCount: capabilities.minImageCount + 1,

		.imageFormat = swapchain._surface_format.format,
		.imageColorSpace = swapchain._surface_format.colorSpace,
		.imageExtent = (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) ?
			capabilities.currentExtent:
			VkExtent2D {
				std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, swapchain._extent.width)),
				std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, swapchain._extent.height))
			},
		.imageArrayLayers = 1,
		.imageUsage = swapchain._image_usage,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = swapchain._present_mode,
		
		// TODO: user defined clipping state
		.clipped = VK_TRUE,
		
		// TODO: remains to be tested
		.oldSwapchain = VK_NULL_HANDLE,
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
	
	if (vkCreateSwapchainKHR(this->_device, &create_info, VK_NULL_HANDLE, &swapchain._swapchain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swapchain!");
	}
	dloggln("Created Swapchain");
	
	
	// ImageViews
	
	vkGetSwapchainImagesKHR(this->_device, swapchain._swapchain, &swapchain._image_count, VK_NULL_HANDLE);
	swapchain._images = new VkImage[swapchain._image_count];
	swapchain._image_views = new VkImageView[swapchain._image_count];
	vkGetSwapchainImagesKHR(this->_device, swapchain._swapchain, &swapchain._image_count, swapchain._images);

	for (size_t i = 0; i < swapchain._image_count; i++) {
		VkImageViewCreateInfo viewInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchain._images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchain._surface_format.format,

			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		if (vkCreateImageView(this->_device, &viewInfo, VK_NULL_HANDLE, &swapchain._image_views[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
	}
	dloggln("ImageViews Created");
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
	dloggln("Pipeline Layout Created");
	
	deletion_queue.push([=, this]{
		vkDestroyPipelineLayout(this->_device, pipeline_layout._pipeline_layout, VK_NULL_HANDLE);
		dloggln("Pipeline Layout Destroyed");
	});
	
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
	dloggln("ShaderModule Created - ", filename);
	
	deletion_queue.push([=]{
		vkDestroyShaderModule(this->_device, shader_module._shader_module, VK_NULL_HANDLE);
		dloggln("Shader Module Destroyed");
	});

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
		.layout = pipeline_layout,
		.renderPass = render_pass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
	};

	if (vkCreateGraphicsPipelines(this->_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("graphics pipeline creation failed!");
	}
	dloggln("Graphics Pipeline Created");

	deletion_queue.push([=, this]{
		vkDestroyPipeline(this->_device, pipeline._pipeline, VK_NULL_HANDLE);
		dloggln("Graphics Pipeline Destroyed");
	});
	
	return pipeline;
}

lvk_pipeline lvk_device::create_compute_pipeline(const VkPipelineLayout pipeline_layout, const VkPipelineShaderStageCreateInfo stage_info) {
	lvk_pipeline pipeline = {
		._pipeline = VK_NULL_HANDLE,
		// .type = VK_PIPELINE_BIND_POINT_COMPUTE,
	};

	VkComputePipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = stage_info,
		.layout = pipeline_layout,
	};

	if (vkCreateComputePipelines(this->_device, VK_NULL_HANDLE, 1, &pipeline_info, VK_NULL_HANDLE, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("compute pipeline creation failed!");
	}
	dloggln("Compute Pipeline Created");
	
	deletion_queue.push([=]{
		vkDestroyPipeline(this->_device, pipeline._pipeline, VK_NULL_HANDLE);
		dloggln("Compute Pipeline Destroyed");
	});
	
	return pipeline;
}


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


void lvk_device::upload(const VmaAllocation allocation, const VkDeviceSize size, const void* data = nullptr) const {
	void* _data = nullptr;
	vmaMapMemory(this->allocator, allocation, &_data);
	
	memcpy(_data, data, size);
	
	vmaUnmapMemory(this->allocator, allocation);
}


lvk_buffer lvk_device::create_buffer(const VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, const VkDeviceSize size, const void* data) {
	lvk_buffer buffer = {
		._buffer = VK_NULL_HANDLE,
		._allocation = VK_NULL_HANDLE,
		._allocated_size = size,
		._usage = buffer_usage,
		._memory_usage = memory_usage,
		// .allocator = this
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
	if (vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &buffer._buffer, &buffer._allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	dloggln("Buffer Created");
	
	if (data != nullptr) {
		upload(buffer, size, data);
	}

	deletion_queue.push([=, this]{
		vmaDestroyBuffer(allocator, buffer._buffer, buffer._allocation);
		dloggln("Buffer Destroyed");
	});

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

		if (staging_buffer._buffer == VK_NULL_HANDLE) {
			staging_buffer = this->create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, size, data);
		} else {
			if (staging_buffer._allocated_size < buffer._allocated_size) {
				throw std::runtime_error("ERROR: staging buffer allocated size does not match the data size!");
			}
		}

		this->imm_submit([&](VkCommandBuffer cmd) {
			static_cast<lvk_command_buffer>(cmd).copy_buffer_to_buffer(staging_buffer, buffer, {
				{
					.srcOffset = 0,
					.dstOffset = 0,
					.size = size
				}
			});
		});
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

	VkImageCreateInfo createInfo = {
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

	VmaAllocationCreateInfo allocationInfo = {
		.usage = memory_usage,
		.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	};

	if (vmaCreateImage(allocator, &createInfo, &allocationInfo, &image._image, &image._allocation, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("image creation failed!");
	}
	dloggln("Image Created - ", image._image);

	deletion_queue.push([=, this]{
		vmaDestroyImage(this->allocator, image._image, image._allocation);
		dloggln("Image Destroyed");
	});

	return image;
}




// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


lvk_image_view lvk_device::create_image_view(const lvk_image& image, VkImageAspectFlags aspect_flag, VkImageViewType image_view_type) {
	lvk_image_view image_view = {
		._image_view = VK_NULL_HANDLE,
	};

	VkImageViewCreateInfo createInfo = lvk::info::image_view(image._image, image._format, aspect_flag, image_view_type);

	if (vkCreateImageView(this->_device, &createInfo, VK_NULL_HANDLE, &image_view._image_view) != VK_SUCCESS) {
		throw std::runtime_error("image_view creation failed!");
	}
	dloggln("ImageView Created - ", image_view._image_view);

	deletion_queue.push([=, this]{
		vkDestroyImageView(this->_device, image_view._image_view, VK_NULL_HANDLE);
		dloggln("ImageView Destroyed");
	});

	return image_view;
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET LAYOUT
// |--------------------------------------------------


lvk_descriptor_set_layout lvk_device::create_descriptor_set_layout(const VkDescriptorSetLayoutBinding* bindings, const uint32_t binding_count) {
	lvk_descriptor_set_layout descriptor_set_layout = {
		._descriptor_set_layout = VK_NULL_HANDLE,
		// .device = this
	};

	VkDescriptorSetLayoutCreateInfo set_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = binding_count,
		.pBindings = bindings,
	};

	if (vkCreateDescriptorSetLayout(_device, &set_info, VK_NULL_HANDLE, &descriptor_set_layout._descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("descriptor_set_layout creation failed!");
	}
	dloggln("DescriptorSetLayout Created");

	deletion_queue.push([=, this]() {
		vkDestroyDescriptorSetLayout(_device, descriptor_set_layout._descriptor_set_layout, VK_NULL_HANDLE);
		dloggln("DescriptorSetLayout Destroyed");
	});

	return descriptor_set_layout;
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


lvk_descriptor_set lvk_device::create_descriptor_set(const lvk_descriptor_pool& descriptor_pool, const lvk_descriptor_set_layout& descriptor_set_layout) {
	lvk_descriptor_set descriptor_set = {
		._descriptor_set = VK_NULL_HANDLE,
	};

	VkDescriptorSetAllocateInfo allocate_info ={
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = descriptor_pool._descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptor_set_layout._descriptor_set_layout,
	};

	if (vkAllocateDescriptorSets(this->_device, &allocate_info, &descriptor_set._descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("descriptor_set allocation failed!");
	}
	dloggln("Description Set Allocated");

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
	dloggln("DescriptorPool Created");

	deletion_queue.push([=](){
		vkDestroyDescriptorPool(_device, descriptor_pool._descriptor_pool, VK_NULL_HANDLE);
		dloggln("DescriptorSetLayout Destroyed");	
	});

	return descriptor_pool;
}

void lvk_device::clear_descriptor_pool(const lvk_descriptor_pool& descriptor_pool) const {
	vkResetDescriptorPool(this->_device, descriptor_pool._descriptor_pool, 0);
}

void lvk_device::destroy_descriptor_pool(const lvk_descriptor_pool& descriptor_pool) const {
	vkDestroyDescriptorPool(this->_device, descriptor_pool._descriptor_pool, VK_NULL_HANDLE);
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

	VkRenderPassCreateInfo createInfo = {
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

	if (vkCreateRenderPass(_device, &createInfo, VK_NULL_HANDLE, &render_pass._render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create renderpass!");
	}
	dloggln("RenderPass Created");
	
	deletion_queue.push([=, this]{
		vkDestroyRenderPass(_device, render_pass._render_pass, VK_NULL_HANDLE);
		dloggln("RenderPass Destroyed");
	});

	return render_pass;
}


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


lvk_framebuffer lvk_device::create_framebuffer(const lvk_render_pass& render_pass, const VkExtent2D extent, const VkImageView* image_views, const uint32_t image_views_count) {
	lvk_framebuffer framebuffer = {
		._framebuffer = VK_NULL_HANDLE,
		._extent = extent
	};
	
	VkFramebufferCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.flags = 0,
		.renderPass = render_pass._render_pass,
		.attachmentCount = image_views_count,
		.pAttachments = image_views,
		.width = extent.width,
		.height = extent.height,
		.layers = 1,
	};
	
	if (vkCreateFramebuffer(this->_device, &createInfo, VK_NULL_HANDLE, &framebuffer._framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer");
	}
	dloggln("Framebuffer Created");
	
	deletion_queue.push([=, this]{
		vkDestroyFramebuffer(this->_device, framebuffer._framebuffer, VK_NULL_HANDLE);
		dloggln("Framebuffer Destroyed");
	});

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
	dloggln("Framebuffer Created");
	
	deletion_queue.push([=, this]{
		vkDestroySampler(this->_device, sampler._sampler, VK_NULL_HANDLE);
		dloggln("Framebuffer Destroyed");
	});

	return sampler;
}

void lvk_device::destroyer::flush() const {
	for (auto function = deletion_queue.rbegin(); function != deletion_queue.rend(); function++) {
		(*function)();
	}
}

void lvk_device::destroyer::pop() {
	(*deletion_queue.rend())();
	deletion_queue.pop_back();
}

void lvk_device::destroyer::push(std::function<void()>&& function) {
	deletion_queue.push_back(function);
}
