#pragma once

#include "lvk/types.h"
#include "vk_mem_alloc.h"

// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


struct lvk_buffer {
	VkBuffer _buffer;
	VmaAllocation _allocation;
	
	std::size_t _allocated_size;
	VkBufferUsageFlagBits _usage;

	const lvk_allocator* allocator;

	void upload(const void* data, const std::size_t size) const;

	template <typename T> inline void upload(const T& data) const { upload(&data, sizeof(T)); }
};


// |--------------------------------------------------
// ----------------> ALLOCATOR
// |--------------------------------------------------


struct lvk_allocator {
	VmaAllocator _allocator;
	
	const lvk_device* device;

	lvk::deletion_queue* deletion_queue;
	
	lvk_buffer init_buffer(VkBufferUsageFlagBits buffer_usage, VmaMemoryUsage memory_usage, const void* data, const std::size_t size);

	lvk_buffer init_vertex_buffer(const void* data, const std::size_t size);

	template <typename T, std::size_t N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_buffer init_vertex_buffer(const T (&data)[N]) noexcept {
		return init_vertex_buffer(data, sizeof(T) * N);
	}

	lvk_buffer init_uniform_buffer(const void* data, const std::size_t size);

	template <typename T> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_buffer init_uniform_buffer(const T* data = nullptr) noexcept {
		return init_uniform_buffer(data, sizeof(T));
	}

	lvk_image init_image(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageType image_type);
};


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------


struct lvk_image {
	VkImage _image;
	VmaAllocation _allocation;
	
	VkFormat _format;
	VkImageType _image_type;
	VkExtent3D _extent;
	VkImageUsageFlags _usage;
	
	const lvk_allocator* allocator;
	const lvk_device* device;

	lvk::deletion_queue* deletion_queue;
	
	lvk_image_view init_image_view(VkImageAspectFlags aspect_flag, VkImageViewType image_view_type);
};


// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


struct lvk_image_view {
	VkImageView _image_view;

	const lvk_image* image;
};