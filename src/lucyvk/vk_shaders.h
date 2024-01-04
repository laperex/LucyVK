#include "lucyvk/vk_types.h"
#include <vulkan/vulkan_core.h>



// ! |--------------------------------------------------
// ! ----------------> SHADER ( OBSELETE SHOULD BE REMOVED AFTER TESTING )
// ! |--------------------------------------------------

struct lvk_shader_module {
	VkShaderModule _shader_module;

	const VkShaderStageFlagBits _stage;

	~lvk_shader_module();

	const lvk_device* device;
};
