#include "lucyvk/vk_command.h"
#include "lucyvk/vk_function.h"

#include "lucyvk/vk_physical_device.h"
#include "lucyvk/vk_device.h"
#include "lucyvk/vk_render_pass.h"
#include "lucyvk/vk_alloc.h"

#include "util/logger.h"
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

void lvk_command_buffer::blit_image_to_image(VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size) {
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