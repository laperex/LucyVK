#include "lucyvk/command.h"
#include "lucyvk/create_info.h"
#include "lucyvk/functions.h"

#include "lucyvk/physical_device.h"
#include "lucyvk/logical_device.h"
#include "lucyvk/render_pass.h"
#include "lucyvk/memory.h"

#include "lucyio/logger.h"
#include <stdexcept>


// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


lvk_command_pool lvk_device::init_command_pool() {
	return init_command_pool(physical_device->_queue_family_indices.graphics.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

lvk_command_pool lvk_device::init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
	lvk_command_pool command_pool = {
		.instance = instance,
		.physical_device = physical_device,
		.device = this,
		.deletion_queue = &deletion_queue,
	};

	VkCommandPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = flags,
		.queueFamilyIndex = queue_family_index,
	};

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

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

		.commandPool = _command_pool,
		.level = level,
		.commandBufferCount = 1,
	};

	if (vkAllocateCommandBuffers(device->_device, &allocate_info, &command_buffer._command_buffer) != VK_SUCCESS) {
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

void lvk_command_buffer::bind_pipeline(const lvk_pipeline* pipeline) {
	vkCmdBindPipeline(_command_buffer, pipeline->type, pipeline->_pipeline);
}

void lvk_command_buffer::bind_descriptor_set(const lvk_pipeline* pipeline, const lvk_descriptor_set* descriptor_set, const uint32_t descriptor_set_count) {
	vkCmdBindDescriptorSets(_command_buffer, pipeline->type, pipeline->pipeline_layout->_pipeline_layout, 0, descriptor_set_count, &descriptor_set->_descriptor_set, 0, VK_NULL_HANDLE);
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

// void lvk_command_buffer::transition_image_imme(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) {
//     // VkImageAspectFlags aspect_mask = (new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

//     // VkImageMemoryBarrier image_barrier {
// 	// 	.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,

// 	// 	.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT,
		
// 	// 	.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT,
		
// 	// 	.oldLayout = current_layout,
// 	// 	.newLayout = new_layout,
		
// 	// 	.image = image,
		
// 	// 	.subresourceRange = lvk::image_subresource_range(aspect_mask),
// 	// };

//     // VkDependencyInfo dependency_info {
// 	// 	.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,

// 	// 	.imageMemoryBarrierCount = 1,
// 	// 	.pImageMemoryBarriers = &image_barrier
// 	// };

//     // vkCmdPipelineBarrier2(_command_buffer, &dependency_info);
// }

void lvk_command_buffer::transition_image(const lvk_image* image, VkImageLayout current_layout, VkImageLayout new_layout) {
	return transition_image(image->_image, current_layout, new_layout);
}

void lvk_command_buffer::copy_image_to_image(VkImage source, VkImage destination, VkImageLayout source_layout, VkImageLayout destination_layout, VkExtent2D src_size, VkExtent2D dst_size) {
	VkImageCopy2 copy_region = {
		
	};

	VkCopyImageInfo2 copy_info = {
		.sType = VK_STRUCTURE_TYPE_COPY_IMAGE_INFO_2,
		.srcImage = source,
		.srcImageLayout = source_layout,

		.dstImage = destination,
		.dstImageLayout = destination_layout,

		.regionCount = 1,
		.pRegions = &copy_region
	};
	
	vkCmdCopyImage2(_command_buffer, &copy_info);
}

void lvk_command_buffer::dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) {
	vkCmdDispatch(_command_buffer, group_count_x, group_count_y, group_count_z);
}

void lvk_command_buffer::blit_image_to_image(VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size) {
	VkImageBlit2 blit_region = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		
		.srcSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},

		.srcOffsets = {
			{}, { static_cast<int32_t>(src_size.width), static_cast<int32_t>(src_size.height), 1 }
		},
		
		.dstSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},

		.dstOffsets = {
			{}, { static_cast<int32_t>(dst_size.width), static_cast<int32_t>(dst_size.height), 1 }
		},
	};

	VkBlitImageInfo2 blit_info = {
		.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		
		.srcImage = source,
		.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,

		.dstImage = destination,
		.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,

		.regionCount = 1,
		.pRegions = &blit_region,

		.filter = VK_FILTER_LINEAR,
	};

	vkCmdBlitImage2(_command_buffer, &blit_info);
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
// ----------------> IMMEDIATE COMMAND BUFFER
// |--------------------------------------------------


lvk_immediate_command_buffer lvk_command_pool::init_immediate_command_buffer() {
	lvk_immediate_command_buffer immediate_command_buffer = {
		.command_pool = this,
		.device = this->device
	};
	
	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

		.commandPool = _command_pool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};
	
	if (vkAllocateCommandBuffers(device->_device, &allocate_info, &immediate_command_buffer._command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("immediate command buffer allocation failed!");
	}
	dloggln("Immediate Command Buffer Allocated: ", &immediate_command_buffer._command_buffer);
	
	VkFenceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
	};
	
	if (vkCreateFence(device->_device, &createInfo, VK_NULL_HANDLE, &immediate_command_buffer._fence) != VK_SUCCESS) {
		throw std::runtime_error("immediate fence creation failed");
	}
	dloggln("Immediate Fence Created");
	
	deletion_queue->push([=]{
		vkDestroyFence(device->_device, immediate_command_buffer._fence, VK_NULL_HANDLE);
		dloggln("Fence Destroyed");
	});

	return immediate_command_buffer;
}

void lvk_immediate_command_buffer::transition_image(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout) const {
	VkCommandBufferBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		.pInheritanceInfo = VK_NULL_HANDLE
	};

	vkBeginCommandBuffer(_command_buffer, &begin_info);
	
	{
		// VkImageSubresourceRange range = {
		// 	.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		// 	.baseMipLevel = 0,
		// 	.levelCount = 1,
		// 	.baseArrayLayer = 0,
		// 	.layerCount = 1,
		// };

		VkImageMemoryBarrier transfer_image_barrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,

			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,

			.oldLayout = old_layout,
			.newLayout = new_layout,

			.image = image,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		//barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &transfer_image_barrier);
	}

	vkEndCommandBuffer(_command_buffer);

	VkCommandBufferSubmitInfo cmdinfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
		.commandBuffer = _command_buffer,
	};
	
	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		
		.commandBufferCount = 1,
		.pCommandBuffers = &_command_buffer,
	};

	device->submit(&submit_info, 1, _fence, LVK_TIMEOUT);

	vkWaitForFences(device->_device, 1, &_fence, true, LVK_TIMEOUT);
	vkResetFences(device->_device, 1, &_fence);
	vkResetCommandBuffer(_command_buffer, 0);
}