#pragma once


#include "lucyvk/types.h"
#include <vulkan/vulkan_core.h>



// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


struct lvk_framebuffer {
	VkFramebuffer _framebuffer;
	VkExtent2D _extent;

	// const lvk_render_pass* render_pass;
	// const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


struct lvk_render_pass {
	VkRenderPass _render_pass;

	// const lvk_device* device;
	// // const lvk_physical_device* physical_device;
	// const lvk_instance* instance;
	
	// lvk::deletion_queue* deletion_queue;

	// lvk_framebuffer init_framebuffer(const VkExtent2D extent, const VkImageView* image_views, const uint32_t image_views_count);

	// template <std::size_t _iv_N> [[nodiscard, __gnu__::__always_inline__]]
	// constexpr lvk_framebuffer init_framebuffer(const VkExtent2D extent, const VkImageView (&image_views)[_iv_N]) noexcept {
	// 	return init_framebuffer(extent, image_views, _iv_N);
	// }
};

// struct lvk_render_pass_config {
// 	void add(VkAttachmentDescription attachment);
// };
