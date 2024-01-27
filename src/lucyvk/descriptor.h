#pragma once

#include <vulkan/vulkan_core.h>
#include "lucyvk/types.h"



// |--------------------------------------------------
// ----------------> DESCRIPTOR SET LAYOUT
// |--------------------------------------------------


struct lvk_descriptor_set_layout {
	VkDescriptorSetLayout _descriptor_set_layout;
	
	const lvk_device* device;
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR POOL
// |--------------------------------------------------


struct lvk_descriptor_pool {
	VkDescriptorPool _descriptor_pool;
	
	const lvk_device* device;
	
	lvk_descriptor_set init_descriptor_set(const lvk_descriptor_set_layout* descriptor_set_layout);
	void clear() const;

	void destroy() const;
};


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


struct lvk_descriptor_set {
	VkDescriptorSet _descriptor_set;
	
	const lvk_descriptor_pool* descriptor_pool;
	const lvk_device* device;
	
	void update(uint32_t binding, const lvk_buffer* buffer, VkDescriptorType type, const std::size_t offset = 0) const;
	void update(uint32_t binding, const lvk_image_view* image_view, VkDescriptorType type, const std::size_t offset = 0) const;
};