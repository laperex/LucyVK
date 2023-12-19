#pragma once

#include <lucyvk/types.h>

namespace lucyvk {
	struct Swapchain {
		const Device& device;
		
		Swapchain(lucyvk::Device& device);

		void Initialize();
	};
}