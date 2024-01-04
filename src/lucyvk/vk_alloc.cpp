#define VMA_IMPLEMENTATION

#include "lucyvk/vk_alloc.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_device.h"
#include "lucyvk/vk_info.h"
#include "lucyvk/vk_physical_device.h"
#include "lucyvk/vk_instance.h"

#include "util/logger.h"
#include <stdexcept>



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