#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "lucyvk/vk_static.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_info.h"

#include "lucyvk/vk_instance.h"
#include "lucyvk/vk_device.h"
#include "lucyvk/vk_physical_device.h"

#include "util/logger.h"
#include <SDL_vulkan.h>
#include <cassert>
#include <iostream>
#include <ostream>
#include <set>
#include <unordered_set>


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


lvk_swapchain lvk_device::init_swapchain(uint32_t width, uint32_t height, VkImageUsageFlags image_usage_flags, VkSurfaceFormatKHR surface_format) {
	const auto& capabilities = physical_device->_swapchain_support_details.capabilities;

	lvk_swapchain swapchain = {
		._swapchain = VK_NULL_HANDLE,
		._extent = VkExtent2D { width, height },
		._surface_format = surface_format,
		._present_mode = VK_PRESENT_MODE_FIFO_KHR,
		._image_usage = image_usage_flags,
		.device = this,
		.physical_device = physical_device,
		.instance = instance
	};

	// TODO: support for more presentMode types
	// * VK_PRESENT_MODE_IMMEDIATE_KHR = Not suggested causes tearing
	// * VK_PRESENT_MODE_MAILBOX_KHR = Mailbox
	// * VK_PRESENT_MODE_FIFO_KHR = V-sync
	// * VK_PRESENT_MODE_FIFO_RELAXED_KHR
	// * VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR
	// * VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR

	for (const auto& availablePresentMode: physical_device->_swapchain_support_details.present_modes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			swapchain._present_mode = availablePresentMode;
			break;
		}
		if (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
			swapchain._present_mode = availablePresentMode;
			break;
		}
	}
	
	swapchain.recreate(width, height);
	
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

bool lvk_swapchain::recreate(const uint32_t width, const uint32_t height) {
	if (_image_count) {
		for (int i = 0; i < _image_count; i++) {
			vkDestroyImageView(device->_device, _image_views[i], VK_NULL_HANDLE);
		}
		dloggln("ImageViews Destroyed");
	}

	if (_swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device->_device, _swapchain, VK_NULL_HANDLE);
		dloggln("Swapchain Destroyed");
	}

	this->_extent.width = width;
	this->_extent.height = height;

	const auto& present_modes = physical_device->_swapchain_support_details.present_modes;
	const auto& capabilities = physical_device->_swapchain_support_details.capabilities;

	VkSwapchainCreateInfoKHR createInfo = {};
	{
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = instance->_surface;

		// TF ???
		createInfo.minImageCount = (capabilities.maxImageCount > 0 && capabilities.minImageCount + 1 > capabilities.maxImageCount) ? capabilities.maxImageCount: capabilities.minImageCount + 1;

		createInfo.imageFormat = this->_surface_format.format;
		createInfo.imageColorSpace = this->_surface_format.colorSpace;
		createInfo.imageExtent = (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) ?
			capabilities.currentExtent:
			VkExtent2D {
				std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, this->_extent.width)),
				std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, this->_extent.height))
			};
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = _image_usage;

		// TODO: better approach
		uint32_t queueFamilyIndices[] = {
			physical_device->_queue_family_indices.graphics.value(),
			physical_device->_queue_family_indices.present.value()
		};

		// TODO: Sharing Mode is always exclusive in lvk_buffer. Therefore only one queue is possible
		if (physical_device->_queue_family_indices.present == physical_device->_queue_family_indices.graphics) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		} else {
			throw std::runtime_error("VK_SHARING_MODE_CONCURRENT is not implemented yet");
			
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = std::size(queueFamilyIndices);
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = this->_present_mode;

		// TODO: user defined clipping state
		createInfo.clipped = VK_TRUE;

		// TODO: remains to be tested
		createInfo.oldSwapchain = VK_NULL_HANDLE;


		if (vkCreateSwapchainKHR(this->device->_device, &createInfo, VK_NULL_HANDLE, &this->_swapchain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swapchain!");
		}
		dloggln("Created Swapchain");
	}
	
	// ImageViews
	
	vkGetSwapchainImagesKHR(this->device->_device, this->_swapchain, &_image_count, VK_NULL_HANDLE);
	_images = new VkImage[_image_count];
	_image_views = new VkImageView[_image_count];
	vkGetSwapchainImagesKHR(this->device->_device, this->_swapchain, &_image_count, _images);

	for (size_t i = 0; i < _image_count; i++) {
		VkImageViewCreateInfo viewInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = _images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = this->_surface_format.format,

			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		if (vkCreateImageView(this->device->_device, &viewInfo, VK_NULL_HANDLE, &this->_image_views[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
	}
	dloggln("ImageViews Created");

	return true;
}

VkResult lvk_swapchain::acquire_next_image(uint32_t* index, VkSemaphore semaphore, VkFence fence, const uint64_t timeout) {
	return vkAcquireNextImageKHR(device->_device, _swapchain, timeout, semaphore, fence, index);
}


// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


lvk_command_pool lvk_device::init_command_pool() {
	return init_command_pool(physical_device->_queue_family_indices.graphics.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

lvk_command_pool lvk_device::init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
	lvk_command_pool command_pool = {};
	
	command_pool.device = this;
	command_pool.physical_device = this->physical_device;
	command_pool.instance = this->instance;
	command_pool.deletion_queue = &deletion_queue;
	
	VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    poolInfo.queueFamilyIndex = queue_family_index;
    poolInfo.flags = flags;

    if (vkCreateCommandPool(_device, &poolInfo, VK_NULL_HANDLE, &command_pool._command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
	
	deletion_queue.push([=]{
		vkDestroyCommandPool(_device, command_pool._command_pool, VK_NULL_HANDLE);
		dloggln("Command Pool Destroyed");
	});

	return command_pool;
}


// |--------------------------------------------------
// ----------------> COMMAND BUFFER
// |--------------------------------------------------


lvk_command_buffer lvk_command_pool::init_command_buffer(VkCommandBufferLevel level) {
	lvk_command_buffer command_buffer = {
		VK_NULL_HANDLE,
		this,
		device
	};

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = _command_pool;
	allocateInfo.level = level;
	allocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device->_device, &allocateInfo, &command_buffer._command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("command buffer allocation failed!");
	}
	dloggln("Command Buffer Allocated: ", &command_buffer._command_buffer);
	
	deletion_queue->push([=]{
		vkFreeCommandBuffers(device->_device, _command_pool, 1, &command_buffer._command_buffer);
		dloggln("Command Buffer Destroyed");
	});
	
	return command_buffer;
}

void lvk_command_buffer::reset(VkCommandBufferResetFlags flags) {
	vkResetCommandBuffer(_command_buffer, flags);
}

void lvk_command_buffer::begin(const VkCommandBufferBeginInfo* beginInfo) {
	vkBeginCommandBuffer(_command_buffer, beginInfo);
}

void lvk_command_buffer::begin(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info) {
	VkCommandBufferBeginInfo cmdBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = flags,
		.pInheritanceInfo = inheritance_info
	};

	vkBeginCommandBuffer(_command_buffer, &cmdBeginInfo);
}

VkCommandBufferSubmitInfo lvk_command_buffer::submit_info() {
	return {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.pNext = nullptr,
		.commandBuffer = _command_buffer,
		.deviceMask = 0,
	};
}

void lvk_command_buffer::bind_pipeline(const lvk_pipeline* pipeline) {
	vkCmdBindPipeline(_command_buffer, pipeline->type, pipeline->_pipeline);
}

void lvk_command_buffer::bind_vertex_buffer(const lvk_buffer* vertex_buffer, const VkDeviceSize offset) {
	vkCmdBindVertexBuffers(_command_buffer, 0, 1, &vertex_buffer->_buffer, &offset);
}

void lvk_command_buffer::bind_vertex_buffers(const VkBuffer* vertex_buffers, const VkDeviceSize* offset_array, const uint32_t vertex_buffers_count, const uint32_t first_binding) {
	vkCmdBindVertexBuffers(_command_buffer, first_binding, vertex_buffers_count, vertex_buffers, offset_array);
}

void lvk_command_buffer::transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) {
    VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageMemoryBarrier2 image_barrier {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

		.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
		.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
		.oldLayout = current_layout,
		.newLayout = new_layout,
		.image = image,
		.subresourceRange = lvk::image_subresource_range(aspect_mask),
	};

    VkDependencyInfo dependency_info {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,

		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &image_barrier
	};

    vkCmdPipelineBarrier2(_command_buffer, &dependency_info);
}

void lvk_command_buffer::transition_image(const lvk_image* image, VkImageLayout current_layout, VkImageLayout new_layout) {
	return transition_image(image->_image, current_layout, new_layout);
}

void lvk_command_buffer::copy_image_to_image(VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size) {
	VkImageBlit2 blitRegion = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
	};

	blitRegion.srcOffsets[1].x = src_size.width;
	blitRegion.srcOffsets[1].y = src_size.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dst_size.width;
	blitRegion.dstOffsets[1].y = dst_size.height;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blitInfo = {
		.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
	};

	blitInfo.dstImage = destination;
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.srcImage = source;
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

	vkCmdBlitImage2(_command_buffer, &blitInfo);
}

void lvk_command_buffer::end() {
	vkEndCommandBuffer(_command_buffer);
}

void lvk_command_buffer::begin_render_pass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents subpass_contents) {
	vkCmdBeginRenderPass(_command_buffer, beginInfo, subpass_contents);
}

void lvk_command_buffer::begin_render_pass(const lvk_framebuffer* framebuffer, const VkSubpassContents subpass_contents, const VkClearValue* clear_values, const uint32_t clear_value_count) {
	VkRenderPassBeginInfo beginInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		VK_NULL_HANDLE,
		framebuffer->render_pass->_render_pass,
		framebuffer->_framebuffer,
		{ { 0, 0 }, framebuffer->_extent },
		clear_value_count,
		clear_values,
	};

	begin_render_pass(&beginInfo, subpass_contents);
}

void lvk_command_buffer::end_render_pass() {
	vkCmdEndRenderPass(_command_buffer);
}


// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


lvk_render_pass lvk_device::init_default_render_pass(VkFormat format) {	
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

	return init_render_pass(attachments, subpasses, dependency);
}

template <std::size_t _ad_N, std::size_t _sdn_N, std::size_t _sdc_N>
lvk_render_pass lvk_device::init_render_pass(const VkAttachmentDescription (&attachment)[_ad_N], const VkSubpassDescription (&subpass)[_sdn_N], const VkSubpassDependency (&dependency)[_sdc_N], bool enable_transform) {
	return init_render_pass(attachment, _ad_N, subpass, _sdn_N, dependency, _sdc_N);
}

lvk_render_pass lvk_device::init_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform) {
	lvk_render_pass render_pass = {
		VK_NULL_HANDLE,
		this,
		physical_device,
		instance,
		&deletion_queue
	};

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = (enable_transform) * VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
	createInfo.attachmentCount = attachment_count;
	createInfo.pAttachments = attachment;
	createInfo.subpassCount = subpass_count;
	createInfo.pSubpasses = subpass;
	createInfo.dependencyCount = dependency_count;
	createInfo.pDependencies = dependency;

	if (vkCreateRenderPass(_device, &createInfo, VK_NULL_HANDLE, &render_pass._render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create renderpass!");
	}
	dloggln("RenderPass Created");
	
	deletion_queue.push([=]{
		vkDestroyRenderPass(_device, render_pass._render_pass, VK_NULL_HANDLE);
		dloggln("RenderPass Destroyed");
	});

	return render_pass;
}


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


lvk_framebuffer lvk_render_pass::init_framebuffer(const VkExtent2D extent, const VkImageView* image_views, const uint32_t image_views_count) {
	lvk_framebuffer framebuffer = {
		VK_NULL_HANDLE,
		extent,
		this,
		device
	};

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.renderPass = _render_pass;
	createInfo.width = extent.width;
	createInfo.height = extent.height;
	createInfo.attachmentCount = image_views_count;
	createInfo.pAttachments = image_views;
	createInfo.layers = 1;

	if (vkCreateFramebuffer(device->_device, &createInfo, VK_NULL_HANDLE, &framebuffer._framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer");
	}
	dloggln("Framebuffer Created");
	
	deletion_queue->push([=]{
		vkDestroyFramebuffer(device->_device, framebuffer._framebuffer, VK_NULL_HANDLE);
		dloggln("Framebuffer Destroyed");
	});

	return framebuffer;
}


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


lvk_semaphore lvk_device::init_semaphore() {
	lvk_semaphore semaphore = {
		VK_NULL_HANDLE,
		this
	};

	VkSemaphoreCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		VK_NULL_HANDLE,
		0
	};
	
	if (vkCreateSemaphore(_device, &createInfo, VK_NULL_HANDLE, &semaphore._semaphore) != VK_SUCCESS) {
		throw std::runtime_error("semaphore creation failed");
	}
	dloggln("Semaphore Created");

	deletion_queue.push([=]{
		vkDestroySemaphore(_device, semaphore._semaphore, VK_NULL_HANDLE);
		dloggln("Semaphore Destroyed");
	});
	
	return semaphore;
}


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


lvk_fence lvk_device::init_fence(VkFenceCreateFlags flags) {
	lvk_fence fence = {
		VK_NULL_HANDLE,
		this
	};

	VkFenceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		VK_NULL_HANDLE,
		flags
	};

	if (vkCreateFence(_device, &createInfo, VK_NULL_HANDLE, &fence._fence) != VK_SUCCESS) {
		throw std::runtime_error("fence creation failed");
	}
	dloggln("Fence Created");
	
	deletion_queue.push([=]{
		vkDestroyFence(_device, fence._fence, VK_NULL_HANDLE);
		dloggln("Fence Destroyed");
	});

	return fence;
}

VkResult lvk_fence::wait(uint64_t timeout) const {
	return vkWaitForFences(device->_device, 1, &_fence, false, timeout);
}

VkResult lvk_fence::reset() const {
	return vkResetFences(device->_device, 1, &_fence);
}


// ! |--------------------------------------------------
// ! ----------------> SHADER ( OBSELETE SHOULD BE REMOVED AFTER TESTING )
// ! |--------------------------------------------------


lvk_shader_module lvk_device::init_shader_module(VkShaderStageFlagBits stage, const char* filename) {
	lvk_shader_module shader_module = {
		VK_NULL_HANDLE,
		stage,
		this
	};
	
	VkShaderModuleCreateInfo info = lvk::info::shader_module(filename);

	assert(info.codeSize);

	if (vkCreateShaderModule(_device, &info, VK_NULL_HANDLE, &shader_module._shader_module) != VK_SUCCESS) {
		throw std::runtime_error(std::string("failed to create shader module! ") + filename);
	}
	dloggln("ShaderModule Created - ", filename);

	return shader_module;
}

lvk_shader_module::~lvk_shader_module()
{
	vkDestroyShaderModule(device->_device, _shader_module, VK_NULL_HANDLE);
	dloggln("Shader Module Destroyed");
}


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


lvk_pipeline_layout lvk_device::init_pipeline_layout(const VkPushConstantRange* push_constant_ranges, uint32_t push_constant_range_count, const VkDescriptorSetLayout* descriptor_set_layouts, uint32_t descriptor_set_layout_count) {
	lvk_pipeline_layout pipeline_layout = {
		._pipeline_layout = VK_NULL_HANDLE,
		.device = this,
		.deletion_queue = &deletion_queue,
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
	
	deletion_queue.push([=]{
		vkDestroyPipelineLayout(_device, pipeline_layout._pipeline_layout, VK_NULL_HANDLE);
		dloggln("Pipeline Layout Destroyed");
	});
	
	return pipeline_layout;
}

lvk_pipeline lvk_pipeline_layout::init_compute_pipeline(const VkPipelineShaderStageCreateInfo stage_info) {
	lvk_pipeline pipeline = {
		._pipeline = VK_NULL_HANDLE,
		.pipeline_layout = this,
		.render_pass = VK_NULL_HANDLE,
		.device = device,
		.type = VK_PIPELINE_BIND_POINT_COMPUTE,
		.deletion_queue = deletion_queue,
	};

	VkComputePipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = stage_info,
		.layout = _pipeline_layout,
	};

	if (vkCreateComputePipelines(device->_device, VK_NULL_HANDLE, 1, &pipeline_info, VK_NULL_HANDLE, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("compute pipeline creation failed!");
	}
	dloggln("Compute Pipeline Created");
	
	deletion_queue->push([=]{
		vkDestroyPipeline(device->_device, pipeline._pipeline, VK_NULL_HANDLE);
		dloggln("Compute Pipeline Destroyed");
	});

	return pipeline;
}

lvk_pipeline lvk_pipeline_layout::init_graphics_pipeline(const lvk_render_pass* render_pass, const lvk::config::graphics_pipeline* config) {
	lvk_pipeline pipeline = {
		._pipeline = VK_NULL_HANDLE,
		.pipeline_layout = this,
		.render_pass = render_pass,
		.device = device,
		.type = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.deletion_queue = deletion_queue,
	};

	// TODO: Support for multiple viewport and scissors
	
	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.pNext = nullptr;

	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &config->viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &config->scissor;

	// TODO: Actual Blending to support Transparent Objects
	
	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.pNext = nullptr;

	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &config->color_blend_attachment;
	
	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = nullptr;

	pipeline_info.stageCount = config->shader_stage_array.size();
	pipeline_info.pStages = config->shader_stage_array.data();
	pipeline_info.pVertexInputState = &config->vertex_input_state;
	pipeline_info.pInputAssemblyState = &config->input_assembly_state;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &config->rasterization_state;
	pipeline_info.pMultisampleState = &config->multisample_state;
	pipeline_info.pDepthStencilState = &config->depth_stencil_state;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = this->_pipeline_layout;
	pipeline_info.renderPass = render_pass->_render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	
	if (vkCreateGraphicsPipelines(device->_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("graphics pipeline creation failed!");
	}
	dloggln("Graphics Pipeline Created");
	
	deletion_queue->push([=]{
		vkDestroyPipeline(device->_device, pipeline._pipeline, VK_NULL_HANDLE);
		dloggln("Graphics Pipeline Destroyed");
	});
	
	return pipeline;
}

lvk_pipeline::~lvk_pipeline()
{
}


// |--------------------------------------------------
// ----------------> ALLOCATOR
// |--------------------------------------------------


lvk_allocator lvk_device::init_allocator() {
	lvk_allocator allocator = {
		VK_NULL_HANDLE,
		this,
		&deletion_queue
	};
	
	VmaAllocatorCreateInfo allocatorInfo = {};

    allocatorInfo.physicalDevice = physical_device->_physical_device;
    allocatorInfo.device = _device;
    allocatorInfo.instance = instance->_instance;

    vmaCreateAllocator(&allocatorInfo, &allocator._allocator);
	dloggln("Allocator Created");
	
	deletion_queue.push([=]{
		vmaDestroyAllocator(allocator._allocator);
		dloggln("Allocator Destroyed");
	});

	return allocator;
}


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


lvk_buffer lvk_allocator::init_buffer(VkBufferUsageFlagBits buffer_usage, VmaMemoryUsage memory_usage, const void* data, const std::size_t size) {
	lvk_buffer buffer = {
		._buffer = VK_NULL_HANDLE,
		._allocation = VK_NULL_HANDLE,
		._allocated_size = size,
		._usage = buffer_usage,
		.allocator = this
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

	dloggln("Buffer Created: ", lvk::to_string(buffer_usage));
	
	if (data != nullptr) {
		buffer.upload(data, size);
	}

	deletion_queue->push([=]{
		vmaDestroyBuffer(_allocator, buffer._buffer, buffer._allocation);
		dloggln("Buffer Destroyed: ", lvk::to_string(buffer._usage));
	});

	return buffer;
}

lvk_buffer lvk_allocator::init_vertex_buffer(const void* data, const std::size_t size) {
	return init_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, data, size);
}

lvk_buffer lvk_allocator::init_uniform_buffer(const void* data, const std::size_t size) {
	return init_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, data, size);
}

void lvk_buffer::upload(const void* data, const std::size_t size) const {
	if (size > _allocated_size) {
		throw std::runtime_error("required size is greater than allocated size!");
	}

	void* _data;
	vmaMapMemory(allocator->_allocator, _allocation, &_data);

	memcpy(_data, data, size);

	vmaUnmapMemory(allocator->_allocator, _allocation);
}


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------


lvk_image lvk_allocator::init_image(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageType image_type) {
	lvk_image image = {
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		format,
		image_type,
		extent,
		usage,
		this,
		device,
		deletion_queue
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
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	if (vmaCreateImage(_allocator, &createInfo, &allocationInfo, &image._image, &image._allocation, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("image creation failed!");
	}
	dloggln("Image Created");

	deletion_queue->push([=]{
		vmaDestroyImage(_allocator, image._image, image._allocation);
		dloggln("Image Destroyed");
	});

	return image;
}


// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


lvk_image_view lvk_image::init_image_view(VkImageAspectFlags aspect_flag, VkImageViewType image_view_type) {
	lvk_image_view image_view = {
		._image_view = VK_NULL_HANDLE,
		.image = this
	};

	VkImageViewCreateInfo createInfo = lvk::info::image_view(_image, _format, aspect_flag, image_view_type);

	if (vkCreateImageView(device->_device, &createInfo, VK_NULL_HANDLE, &image_view._image_view) != VK_SUCCESS) {
		throw std::runtime_error("image_view creation failed!");
	}
	dloggln("ImageView Created");

	deletion_queue->push([=]{
		vkDestroyImageView(device->_device, image_view._image_view, VK_NULL_HANDLE);
		dloggln("ImageView Destroyed");
	});

	return image_view;
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET LAYOUT
// |--------------------------------------------------


lvk_descriptor_set_layout lvk_device::init_descriptor_set_layout(const VkDescriptorSetLayoutBinding* bindings, const uint32_t binding_count) {
	lvk_descriptor_set_layout descriptor_set_layout = {
		._descriptor_set_layout = VK_NULL_HANDLE,
		.device = this
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

	deletion_queue.push([=]() {
		vkDestroyDescriptorSetLayout(_device, descriptor_set_layout._descriptor_set_layout, VK_NULL_HANDLE);
		dloggln("DescriptorSetLayout Destroyed");
	});

	return descriptor_set_layout;
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR POOL
// |--------------------------------------------------


lvk_descriptor_pool lvk_device::init_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize* descriptor_pool_sizes, const uint32_t descriptor_pool_sizes_count) {
	lvk_descriptor_pool descriptor_pool = {
		._descriptor_pool = VK_NULL_HANDLE,
		.device = this
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

void lvk_descriptor_pool::clear() const {
	vkResetDescriptorPool(device->_device, _descriptor_pool, 0);
}

void lvk_descriptor_pool::destroy() const {
	vkDestroyDescriptorPool(device->_device, _descriptor_pool, VK_NULL_HANDLE);
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


lvk_descriptor_set lvk_descriptor_pool::init_descriptor_set(const lvk_descriptor_set_layout* descriptor_set_layout) {
	lvk_descriptor_set descriptor_set = {
		._descriptor_set = VK_NULL_HANDLE,
		.descriptor_pool = this,
		.device = device
	};

	VkDescriptorSetAllocateInfo allocate_info ={
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = _descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptor_set_layout->_descriptor_set_layout,
	};

	if (vkAllocateDescriptorSets(device->_device, &allocate_info, &descriptor_set._descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("descriptor_set allocation failed!");
	}
	dloggln("Description Set Allocated");

	return descriptor_set;
}

void lvk_descriptor_set::update(const lvk_buffer* buffer, VkDescriptorType type, const std::size_t offset) const {
	VkDescriptorBufferInfo buffer_info = {
		.buffer = buffer->_buffer,
		.offset = offset,
		.range = buffer->_allocated_size
	};
	
	VkWriteDescriptorSet write_set = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descriptor_set,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = type,
		.pBufferInfo = &buffer_info,
	};

	vkUpdateDescriptorSets(device->_device, 1, &write_set, 0, VK_NULL_HANDLE);
}

void lvk_descriptor_set::update(const lvk_image_view* image_view, VkDescriptorType type, const std::size_t offset) const {
	VkDescriptorImageInfo image_info = {
		.imageView = image_view->_image_view,
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	};
	
	VkWriteDescriptorSet write_set = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descriptor_set,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = type,
		.pImageInfo = &image_info,
	};

	vkUpdateDescriptorSets(device->_device, 1, &write_set, 0, VK_NULL_HANDLE);
}
