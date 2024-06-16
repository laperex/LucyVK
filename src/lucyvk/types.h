#pragma once

#include "lucyvk/define.h"

#include <SDL_video.h>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <vulkan/vulkan_core.h>

struct lvk_instance;

// struct lvk_physical_device;
struct lvk_device;
using lvk_device_ptr = std::unique_ptr<lvk_device>;
struct lvk_device_inherit {
	const lvk_device* device = VK_NULL_HANDLE;
};
// using lvk_device_make = std::make_unique;

// typedef lvk_device_make std::make_unique;

struct lvk_queue;

struct lvk_command_pool;
struct lvk_command_buffer;
struct lvk_immediate_command;

struct lvk_swapchain;
struct lvk_render_pass;

struct lvk_framebuffer;

struct lvk_semaphore;
struct lvk_fence;

struct lvk_shader_module;

struct lvk_pipeline_layout;
struct lvk_pipeline;

struct lvk_descriptor_set_layout;
struct lvk_descriptor_pool;
struct lvk_descriptor_set;

struct lvk_allocator;
struct lvk_buffer;
struct lvk_image;
struct lvk_image_view;


template <typename M>
struct lvk_instance_destructor {
	std::deque<std::function<void(const M)>> deletion_queue;

	void flush(const M m) const {
		for (auto function = deletion_queue.rbegin(); function != deletion_queue.rend(); function++) {
			(*function)(m);
		}
	}

	template <typename T> void push(T);
};

namespace lvk {
	// template <std::size_t B, std::size_t A>
	// struct vertex_input_description {
	// 	VkVertexInputBindingDescription bindings[B];
	// 	VkVertexInputAttributeDescription attributes[A];

	// 	// VkPipelineVertexInputStateCreateFlags flags = 0;

	// 	// template <typename T>
	// 	// static auto def() -> vertex_input_description<B, A>;
	// };
	
	// template<std::size_t B, std::size_t A>
	// vertex_input_description(const VkVertexInputBindingDescription (&)[B], const VkVertexInputAttributeDescription (&)[A]) -> vertex_input_description<B, A>;

	struct deletion_queue {
		std::deque<std::function<void()>> deletion_queue;

		void flush() const {
			for (auto function = deletion_queue.rbegin(); function != deletion_queue.rend(); function++) {
				(*function)();
			}
		}

		void push(std::function<void()>&& function) {
			deletion_queue.push_back(function);
		}
	};

	struct queue_family_indices {
		// union {
		// 	struct {
				std::optional<uint32_t> graphics;
				std::optional<uint32_t> present;
				std::optional<uint32_t> compute;
				std::optional<uint32_t> transfer;
		// 	};
			
		// 	uint32_t indices[4];
		// };

		operator bool() const {
			return graphics.has_value() && present.has_value() && compute.has_value() && transfer.has_value();
		}

		// const bool unique() const {
		// 	return (graphics.value() == present.value());
		// }
	};

	struct swapchain_support_details {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	typedef std::function<VkPhysicalDevice(const std::vector<VkPhysicalDevice>&, const lvk_instance* instance)> SelectPhysicalDeviceFunction;
}
