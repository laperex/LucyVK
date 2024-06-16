#pragma once

#include "lucyvk/types.h"
#include "lucyvk/handles.h"
#include "vk_mem_alloc.h"


// |--------------------------------------------------
// ----------------> ALLOCATOR
// |--------------------------------------------------


struct lvk_allocator {
	VmaAllocator _allocator;

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