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
	
	void upload(const lvk_buffer& buffer, const VkDeviceSize size, const void* data = nullptr) const;
	template <typename T>
	inline void upload(const lvk_buffer& buffer, const T& data) const {
		upload(buffer, sizeof(T), &data);
	}
	
	lvk_buffer create_buffer(const VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, const VkDeviceSize size, const void* data = nullptr);
	
	
	lvk_buffer create_staging_buffer(const VkDeviceSize size, const void* data = nullptr);
	
	
	lvk_buffer create_index_buffer(const VkDeviceSize size, const void* data = nullptr);
	lvk_buffer create_index_buffer_static();

	template <typename T>
	lvk_buffer create_index_buffer(const std::vector<T>& data) {
		return create_index_buffer(data.size() * sizeof(T), data.data());
	}
	
	
	lvk_buffer create_vertex_buffer(const VkDeviceSize size, const void* data = nullptr);
	lvk_buffer create_vertex_buffer_static();

	template <typename T>
	lvk_buffer create_vertex_buffer(const std::vector<T>& data) {
		return create_vertex_buffer(data.size() * sizeof(T), data.data());
	}

	template <typename T, std::size_t N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_buffer create_vertex_buffer(const T (&data)[N]) noexcept {
		return create_vertex_buffer(sizeof(T) * N, data);
	}


	lvk_buffer create_uniform_buffer(const VkDeviceSize size, const void* data = nullptr);
	lvk_buffer create_uniform_buffer_static();

	template <typename T> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_buffer create_uniform_buffer(const T* data = nullptr) noexcept {
		return create_uniform_buffer(sizeof(T), data);
	}


	lvk_image create_image(VkFormat format, VkImageUsageFlags usage, VkExtent3D extent, VkImageType image_type);
};