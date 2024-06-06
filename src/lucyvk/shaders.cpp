#include "lucyvk/shaders.h"
#include "lucyvk/device.h"
#include "lucyvk/create_info.h"-
#include "lucyio/logger.h"
#include <cassert>
#include <stdexcept>
#include <string>



// ! |--------------------------------------------------
// ! ----------------> SHADER ( OBSELETE SHOULD BE REMOVED AFTER TESTING )
// ! |--------------------------------------------------


lvk_shader_module lvk_device::init_shader_module(VkShaderStageFlagBits stage, const char* filename) {
	lvk_shader_module shader_module = {
		VK_NULL_HANDLE,
		stage,
		this
	};
	
	VkShaderModuleCreateInfo info = lvk::info::shader_module(filename);

	assert(info.codeSize);

	if (vkCreateShaderModule(_device, &info, VK_NULL_HANDLE, &shader_module._shader_module) != VK_SUCCESS) {
		throw std::runtime_error(std::string("failed to create shader module! ") + filename);
	}
	dloggln("ShaderModule Created - ", filename);

	return shader_module;
}

lvk_shader_module::~lvk_shader_module()
{
	vkDestroyShaderModule(device->_device, _shader_module, VK_NULL_HANDLE);
	dloggln("Shader Module Destroyed");
}
