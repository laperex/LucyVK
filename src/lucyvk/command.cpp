#include "lucyvk/command.h"
#include "lucyvk/create_info.h"
#include "lucyvk/functions.h"

// #include "lucyvk/physical_device.h"
#include "lucyvk/device.h"
// #include "lucyvk/render_pass.h"
// #include "lucyvk/memory.h"

#include "lucyio/logger.h"
#include <stdexcept>


// |--------------------------------------------------
// ----------------> COMMAND BUFFER
// |--------------------------------------------------


void lvk_command_buffer::reset(VkCommandBufferResetFlags flags) const {
	vkResetCommandBuffer(_command_buffer, flags);
}

void lvk_command_buffer::begin(const VkCommandBufferBeginInfo* beginInfo) const {
	vkBeginCommandBuffer(_command_buffer, beginInfo);
}

void lvk_command_buffer::begin(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info) const {
	VkCommandBufferBeginInfo cmdBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = flags,
		.pInheritanceInfo = inheritance_info
	};

	vkBeginCommandBuffer(_command_buffer, &cmdBeginInfo);
}

void lvk_command_buffer::bind_pipeline(const VkPipelineBindPoint pipeline_bind_point, const VkPipeline pipeline) const {
	vkCmdBindPipeline(_command_buffer, pipeline_bind_point, pipeline);
}

void lvk_command_buffer::bind_graphics_pipeline(const VkPipeline pipeline) const {
	bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void lvk_command_buffer::bind_compute_pipeline(const VkPipeline pipeline) const {
	bind_pipeline(VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

void lvk_command_buffer::bind_descriptor_set(const VkPipelineBindPoint pipeline_bind_point, const VkPipelineLayout pipeline_layout, const lvk_descriptor_set* descriptor_set, const uint32_t descriptor_set_count, uint32_t first_set) const {
	vkCmdBindDescriptorSets(_command_buffer, pipeline_bind_point, pipeline_layout, first_set, descriptor_set_count, &descriptor_set->_descriptor_set, 0, VK_NULL_HANDLE);
}

void lvk_command_buffer::bind_index_buffer(const VkBuffer index_buffer, const VkDeviceSize offset, const VkIndexType index_type) const {
	vkCmdBindIndexBuffer(_command_buffer, index_buffer, offset, index_type);
}

void lvk_command_buffer::bind_vertex_buffers(const VkBuffer* vertex_buffers, const VkDeviceSize* offset_array, const uint32_t vertex_buffers_count, const uint32_t first_binding) const {
	vkCmdBindVertexBuffers(_command_buffer, first_binding, vertex_buffers_count, vertex_buffers, offset_array);
}


void lvk_command_buffer::transition_image(VkImageMemoryBarrier image_memory_barrier) const {
	// VkImageMemoryBarrier transfer_image_barrier = {
	// 	.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,

	// 	.srcAccessMask = src_access_mask,
	// 	.dstAccessMask = dst_access_flags,

	// 	.oldLayout = old_layout,
	// 	.newLayout = new_layout,

	// 	.image = image,
	// 	.subresourceRange = {
	// 		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
	// 		.baseMipLevel = 0,
	// 		.levelCount = 1,
	// 		.baseArrayLayer = 0,
	// 		.layerCount = 1,
	// 	},
	// };

	vkCmdPipelineBarrier(_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

void lvk_command_buffer::pipeline_barrier(VkPipelineStageFlags src_pipeline_stage, VkPipelineStageFlags dst_pipeline_stage, const VkImageMemoryBarrier image_memory_barrier) const {
	vkCmdPipelineBarrier(_command_buffer, src_pipeline_stage, dst_pipeline_stage, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
}

void lvk_command_buffer::clear_color_image(VkImage image, VkImageLayout image_layout, VkClearColorValue* clear_value_array, uint32_t clear_value_array_size, VkImageSubresourceRange clear_range) const {
	vkCmdClearColorImage(_command_buffer, image, image_layout, clear_value_array, clear_value_array_size, &clear_range);
}

void lvk_command_buffer::clear_color_image(VkImage image, VkImageLayout image_layout, VkClearColorValue clear_value, VkImageAspectFlags aspect_flags) const {
	VkImageSubresourceRange clear_range = lvk::info::image_subresource_range(aspect_flags);

	vkCmdClearColorImage(_command_buffer, image, image_layout, &clear_value, 1, &clear_range);	
}

void lvk_command_buffer::begin_rendering(VkExtent2D extent,
	const VkRenderingAttachmentInfo* color_attachment_array, uint32_t color_attachment_array_size,
	const VkRenderingAttachmentInfo* depth_attachment,
	const VkRenderingAttachmentInfo* stencil_attachment
) const {
	const VkRenderingInfo render_info = {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {
			.offset = { 0, 0 },
			.extent = extent
		},
		.layerCount = 1,

		.colorAttachmentCount = color_attachment_array_size,
		.pColorAttachments = color_attachment_array,
		
		.pDepthAttachment = depth_attachment,
		
		.pStencilAttachment = stencil_attachment
	};

	vkCmdBeginRendering(_command_buffer, &render_info);
}

void lvk_command_buffer::push_constants(const VkPipelineLayout pipeline_layout, const VkShaderStageFlags shader_stage, const uint32_t offset, const uint32_t size, const void* data) const {
	vkCmdPushConstants(_command_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, size, data);
}

void lvk_command_buffer::end_rendering() const {
	vkCmdEndRendering(_command_buffer);
}

// VkSubmitInfo lvk_command_buffer::immediate_transition_image(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const {
// 	return immediate([=]{
// 		transition_image(image, current_layout, new_layout);
// 	});
// }


// VkSubmitInfo lvk_command_buffer::immediate_transition_image2(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const {
// 	return immediate([=]{
// 		transition_image2(image, current_layout, new_layout);
// 	});
// }

void lvk_command_buffer::transition_image2(VkImage image, VkImageLayout current_layout, VkImageLayout new_layout) const {
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
		
		.subresourceRange = lvk::info::image_subresource_range(aspect_mask, 0, VK_REMAINING_MIP_LEVELS, 0, VK_REMAINING_ARRAY_LAYERS),
	};

    VkDependencyInfo dependency_info {
		.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,

		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &image_barrier
	};

    vkCmdPipelineBarrier2(_command_buffer, &dependency_info);
}


void lvk_command_buffer::copy_image_to_image2(VkImage source, VkImage destination, VkImageLayout source_layout, VkImageLayout destination_layout, VkExtent2D src_size, VkExtent2D dst_size) const {
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

void lvk_command_buffer::copy_buffer_to_image(VkBuffer source, VkImage destination, VkExtent3D region) const {
	VkBufferImageCopy copy_region = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,

		.imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageExtent = region,
	};

	// copy the buffer into the image
	vkCmdCopyBufferToImage(_command_buffer, source, destination, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
}

void lvk_command_buffer::copy_buffer(const VkBuffer src_buffer, const VkBuffer dst_buffer, const VkBufferCopy* buffer_copy_array, const uint32_t buffer_copy_array_size) const {
	vkCmdCopyBuffer(_command_buffer, src_buffer, dst_buffer, buffer_copy_array_size, buffer_copy_array);
}

void lvk_command_buffer::dispatch(uint32_t group_count_x, uint32_t group_count_y, uint32_t group_count_z) const {
	vkCmdDispatch(_command_buffer, group_count_x, group_count_y, group_count_z);
}

void lvk_command_buffer::blit_image_to_image2(VkImage source, VkImage destination, VkExtent2D src_size, VkExtent2D dst_size) const {
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

void lvk_command_buffer::set_viewport(const uint32_t first, const VkViewport* viewports, const uint32_t count) const {
	vkCmdSetViewport(_command_buffer, first, count, viewports);
}

void lvk_command_buffer::set_viewport(const VkViewport viewport) const {
	vkCmdSetViewport(_command_buffer, 0, 1, &viewport);
}

void lvk_command_buffer::set_scissor(const uint32_t first, const VkRect2D* scissors, const uint32_t count) const {
	vkCmdSetScissor(_command_buffer, first, count, scissors);
}

void lvk_command_buffer::set_scissor(const VkRect2D scissors) const {
	vkCmdSetScissor(_command_buffer, 0, 1, &scissors);
}

void lvk_command_buffer::end() const {
	vkEndCommandBuffer(_command_buffer);
}

void lvk_command_buffer::begin_render_pass(const VkRenderPassBeginInfo begin_info, const VkSubpassContents subpass_contents) const {
	vkCmdBeginRenderPass(_command_buffer, &begin_info, subpass_contents);
}

void lvk_command_buffer::draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) const {
	vkCmdDrawIndexed(_command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void lvk_command_buffer::draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) const {
	vkCmdDraw(_command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void lvk_command_buffer::begin_render_pass(const VkRenderPass render_pass, const VkFramebuffer framebuffer, const VkExtent2D extent, const VkSubpassContents subpass_contents, const VkClearValue* clear_values, const uint32_t clear_value_count) const {
	VkRenderPassBeginInfo begin_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		
		.renderPass = render_pass,
		.framebuffer = framebuffer,

		.renderArea = {
			.offset = { 0, 0 },
			.extent = extent,
		},
		
		.clearValueCount = clear_value_count,
		.pClearValues = clear_values,
	};

	begin_render_pass(begin_info, subpass_contents);
}

void lvk_command_buffer::end_render_pass() const {
	vkCmdEndRenderPass(_command_buffer);
}





// VkSubmitInfo lvk_command_buffer_immediate::immediate(std::function<void(const lvk_command_buffer&)> function) const {
// 	begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
// 	{
// 		function(*this);
// 	}
// 	end();

// 	return {
// 		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		
// 		.commandBufferCount = 1,
// 		.pCommandBuffers = &_command_buffer,
// 	};
// }