#pragma once

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


// enum LVK_SHADER_STAGE_ {
// 	LVK_SHADER_STAGE_VERTEX_BIT = 0x00000001,					// VK_SHADER_STAGE_VERTEX_BIT,
// 	LVK_SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,		// VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
// 	LVK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,	// VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
// 	LVK_SHADER_STAGE_GEOMETRY_BIT = 0x00000008,					// VK_SHADER_STAGE_GEOMETRY_BIT,
// 	LVK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,					// VK_SHADER_STAGE_FRAGMENT_BIT,
// // 	LVK_SHADER_STAGE_COMPUTE_BIT = 0x00000020,					// VK_SHADER_STAGE_COMPUTE_BIT,
// 	LVK_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,				// VK_SHADER_STAGE_ALL_GRAPHICS,
// };


namespace lvk {
	struct graphics_pipeline_config;
	
	// struct deletion_queue {
	// 	std::deque<std::function<void()>> deletion_queue;

	// 	void flush() const {
	// 		for (auto function = deletion_queue.rbegin(); function != deletion_queue.rend(); function++) {
	// 			(*function)();
	// 		}
	// 	}

	// 	void push(std::function<void()>&& function) {
	// 		deletion_queue.push_back(function);
	// 	}
	// };
	
	struct queue_family_indices {
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;
		std::optional<uint32_t> compute;
		std::optional<uint32_t> sparse_binding;

		operator bool() const {
			return graphics.has_value() && present.has_value();
		}

		const bool unique() const {
			return (graphics.value() == present.value());
		}
	};

	struct swapchain_support_details {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};

	typedef std::function<VkPhysicalDevice(const std::vector<VkPhysicalDevice>&, const lvk_instance* instance)> SelectPhysicalDeviceFunction;

	lvk_instance initialize(const char* name, SDL_Window* sdl_window, bool debug_enable);
}