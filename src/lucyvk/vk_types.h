#pragma once

#include "vk_define.h"

#include <SDL_video.h>
#include <deque>
#include <functional>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

#define LVK_EXCEPT(assertion, message)	\
	if ((assertion) != true)	\
		throw std::runtime_error(message);


#define LVK_TIMEOUT 1000000000

struct lvk_instance;

struct lvk_physical_device;
struct lvk_device;

struct lvk_queue;

struct lvk_command_pool;
struct lvk_command_buffer;

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

namespace lvk {
	struct vertex_input_description {
		std::vector<VkVertexInputBindingDescription> bindings;
		std::vector<VkVertexInputAttributeDescription> attributes;

		VkPipelineVertexInputStateCreateFlags flags = 0;
	};

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