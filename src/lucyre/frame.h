#include <vulkan/vulkan_core.h>


struct lvk_frame {
	VkFence render_fence;

	VkSemaphore present_semaphore;
	VkSemaphore render_semaphore;

	VkCommandBuffer command_buffer;

	uint32_t image_index;


	void initialize(VkDevice device);
};