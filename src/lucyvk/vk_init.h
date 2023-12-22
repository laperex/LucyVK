#pragma once

#include <SDL_video.h>
#include <functional>
#include <optional>
#include <vector>
#include <vulkan/vulkan_core.h>

struct lvk_instance;
struct lvk_physical_device;
struct lvk_device;

namespace lvk {
	struct swapchain;
	
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
// ###################################################
// ################# INSTANCE ########################
// ###################################################

struct lvk_instance {
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkSurfaceKHR _surface;
	VkInstance _instance;

	std::vector<const char*> layers = {};
	
	~lvk_instance();

	bool is_debug_enable();

	lvk_physical_device init_physical_device(lvk::SelectPhysicalDeviceFunction function = nullptr);
};

// ###################################################
// ################# PHYSICAL DEVICE #################
// ###################################################

struct lvk_physical_device {
	VkPhysicalDevice _physical_device;

	VkPhysicalDeviceFeatures _features;
	VkPhysicalDeviceProperties _properties;

	lvk::queue_family_indices _queue_family_indices;
	lvk::swapchain_support_details _swapchain_support_details;

	const lvk_instance* instance;

	lvk_device init_device();

	const VkFormat find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	const uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags property_flags) const;
};

// ###################################################
// ################# DEVICE ##########################
// ###################################################

struct lvk_device {
	VkDevice _device;
	
	VkQueue _graphicsQueue;
	VkQueue _presentQueue;
	
	std::vector<const char*> extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	
	~lvk_device();

	const lvk_instance* instance;
	const lvk_physical_device* physical_device;
};
