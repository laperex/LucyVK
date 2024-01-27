#include "lucyvk/render_pass.h"
#include "lucyvk/logical_device.h"
#include "lucyio/logger.h"



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

	VkFramebufferCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.flags = 0,
		.renderPass = _render_pass,
		.attachmentCount = image_views_count,
		.pAttachments = image_views,
		.width = extent.width,
		.height = extent.height,
		.layers = 1,
	};

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