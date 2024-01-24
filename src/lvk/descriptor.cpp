#include "lvk/descriptor.h"
#include "lvk/logical_device.h"
#include "lvk/memory.h"

#include "util/logger.h"
#include <stdexcept>



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

void lvk_descriptor_set::update(uint32_t binding, const lvk_buffer* buffer, VkDescriptorType type, const std::size_t offset) const {
	VkDescriptorBufferInfo buffer_info = {
		.buffer = buffer->_buffer,
		.offset = offset,
		.range = buffer->_allocated_size
	};
	
	VkWriteDescriptorSet write_set = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descriptor_set,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = type,
		.pBufferInfo = &buffer_info,
	};

	vkUpdateDescriptorSets(device->_device, 1, &write_set, 0, VK_NULL_HANDLE);
}

void lvk_descriptor_set::update(uint32_t binding, const lvk_image_view* image_view, VkDescriptorType type, const std::size_t offset) const {
	VkDescriptorImageInfo image_info = {
		.imageView = image_view->_image_view,
		.imageLayout = VK_IMAGE_LAYOUT_GENERAL,
	};
	
	VkWriteDescriptorSet write_set = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descriptor_set,
		.dstBinding = binding,
		.descriptorCount = 1,
		.descriptorType = type,
		.pImageInfo = &image_info,
	};

	vkUpdateDescriptorSets(device->_device, 1, &write_set, 0, VK_NULL_HANDLE);
}
