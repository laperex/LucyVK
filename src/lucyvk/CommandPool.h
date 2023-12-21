#pragma once

#include <lucyvk/types.h>
#include <vulkan/vulkan_core.h>

namespace lucyvk {
	struct CommandPool {
		VkCommandPool _commandPool;
		
		const Device& device;

		CommandPool(const Device& device);
		~CommandPool();

	private:
		bool Initialize();
		bool Destroy();
	};
}