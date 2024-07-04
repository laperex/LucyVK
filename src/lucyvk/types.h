#pragma once

#include "lucyvk/define.h"

#include <SDL_video.h>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <vulkan/vulkan_core.h>


namespace lvk {
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
		std::optional<uint32_t> transfer;

		operator bool() const {
			return graphics.has_value() && present.has_value() && compute.has_value() && transfer.has_value();
		}
	};

	struct swapchain_support_details {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	};
}
