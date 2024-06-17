
#define VMA_IMPLEMENTATION

#include "lucyvk/memory.h"
// #include "lucyvk/instance.h"
// #include "lucyvk/device.h"
// #include "lucyvk/physical_device.h"
#include "lucyvk/functions.h"
#include "lucyvk/create_info.h"

#include "vk_mem_alloc.h"

#include "lucyio/logger.h"
#include <stdexcept>




// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


lvk_buffer lvk_allocator::create_buffer(VkBufferUsageFlagBits buffer_usage, VmaMemoryUsage memory_usage, const void* data, const std::size_t size) {
	lvk_buffer buffer = {
		._buffer = VK_NULL_HANDLE,
		._allocation = VK_NULL_HANDLE,
		._allocated_size = size,
		._usage = buffer_usage,
		// .allocator = this
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
		upload(buffer, data, size);
	}

	deletion_queue->push([=]{
		vmaDestroyBuffer(_allocator, buffer._buffer, buffer._allocation);
		dloggln("Buffer Destroyed: ", lvk::to_string(buffer._usage));
	});

	return buffer;
}

lvk_buffer lvk_allocator::create_vertex_buffer(const void* data, const std::size_t size) {
	return create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, data, size);
}

lvk_buffer lvk_allocator::create_uniform_buffer(const void* data, const std::size_t size) {
	return create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, data, size);
}

void lvk_allocator::upload(const lvk_buffer& buffer, const void* data, const std::size_t size) const {
	if (size > buffer._allocated_size) {
		throw std::runtime_error("required size is greater than allocated size!");
	}

	void* _data;
	vmaMapMemory(this->_allocator, buffer._allocation, &_data);

	memcpy(_data, data, size);

	vmaUnmapMemory(this->_allocator, buffer._allocation);
}


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------


lvk_image lvk_allocator::create_image(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageType image_type) {
	lvk_image image = {
		._image = VK_NULL_HANDLE,
		._allocation = VK_NULL_HANDLE,

		._format = format,
		._image_type = image_type,
		._extent = extent,
		._usage = usage,
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
		.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	};

	if (vmaCreateImage(_allocator, &createInfo, &allocationInfo, &image._image, &image._allocation, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("image creation failed!");
	}
	dloggln("Image Created - ", image._image);

	deletion_queue->push([=]{
		vmaDestroyImage(_allocator, image._image, image._allocation);
		dloggln("Image Destroyed");
	});

	return image;
}

