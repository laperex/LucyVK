// #include "immediate_command.h"
// #include <lucyvk/device.h>

// void lre::immediate_submit::initialize(lvk_device& device) {
// 	command_pool = device.create_graphics_command_pool();
// 	command_buffer = device.allocate_command_buffer_unique(command_pool);
// 	fence = device.create_fence();
// }

// void lre::immediate_submit::submit(lvk_device& device, std::function<void(const lvk_command_buffer&)>&& function) {
// 	VkResult result = device.submit({ command_buffer.immediate(function) }, fence);
	
// 	device.wait_for_fence(fence);
// 	device.reset_fence(fence);
	
// 	device.reset_command_pool(command_pool);
// }

