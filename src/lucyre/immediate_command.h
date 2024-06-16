#pragma once

#include "lucyvk/command.h"

namespace lre {
	struct immediate_submit {
		lvk_command_buffer command_buffer;
		lvk_command_pool command_pool;
		lvk_fence fence;
		
		void initialize(lvk_device& device);
		
		void submit(lvk_device& device, std::function<void(const lvk_command_buffer&)>&& function);
	};
}