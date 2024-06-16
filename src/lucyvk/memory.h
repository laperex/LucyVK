#pragma once

#include "lucyvk/types.h"
#include "lucyvk/handles.h"
#include "vk_mem_alloc.h"

// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


struct lvk_buffer {
	VkBuffer _buffer;
	VmaAllocation _allocation;
	
	std::size_t _allocated_size;
	VkBufferUsageFlagBits _usage;

	// const lvk_allocator* allocator;


};


// |--------------------------------------------------
// ----------------> ALLOCATOR
// |--------------------------------------------------


struct lvk_allocator {
	VmaAllocator _allocator;
	
	// const lvk_device* device;

	lvk::deletion_queue* deletion_queue;
	
	void upload(const lvk_buffer& buffer, const void* data, const std::size_t size) const;
	template <typename T> inline void upload(const lvk_buffer& buffer, const T& data) const {
		upload(buffer, &data, sizeof(T));
	}
	
	lvk_buffer create_buffer(VkBufferUsageFlagBits buffer_usage, VmaMemoryUsage memory_usage, const void* data, const std::size_t size);

	lvk_buffer create_vertex_buffer(const void* data, const std::size_t size);

	template <typename T, std::size_t N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_buffer create_vertex_buffer(const T (&data)[N]) noexcept {
		return create_vertex_buffer(data, sizeof(T) * N);
	}

	lvk_buffer create_uniform_buffer(const void* data, const std::size_t size);

	template <typename T> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_buffer create_uniform_buffer(const T* data = nullptr) noexcept {
		return create_uniform_buffer(data, sizeof(T));
	}

	lvk_image create_image(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageType image_type);
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
	
	// const lvk_allocator* allocator;
	// const lvk_device* device;

	// lvk::deletion_queue* deletion_queue;
	
	// lvk_image_view init_image_view(VkImageAspectFlags aspect_flag, VkImageViewType image_view_type);
};


// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


struct lvk_image_view {
	VkImageView _image_view;

	// const lvk_image* image;
};