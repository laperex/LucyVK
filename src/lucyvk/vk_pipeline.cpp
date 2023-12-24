#include "lucyvk/vk_pipeline.h"
#include <fstream>


VkShaderModuleCreateInfo lvk::shader_module_create_info(const char* filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error(std::string("failed to open file! ") + filename);
	}

	size_t size = (size_t)file.tellg();
	char* buffer = new char[size];
	
	file.seekg(0);
	file.read(buffer, size);
	
	file.close();
	
	return {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		nullptr,
		0,
		size,
		reinterpret_cast<const uint32_t*>(buffer)
	};
}

VkPipelineShaderStageCreateInfo lvk::shader_stage_create_info(VkShaderStageFlagBits flag_bits, VkShaderModule shader_module, const VkSpecializationInfo* specialization) {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		flag_bits,
		shader_module,
		"main",
		specialization
	};
}

VkPipelineShaderStageCreateInfo lvk::shader_stage_create_info(const lvk_shader_module* shader_module, const VkSpecializationInfo* specialization) {
	return shader_stage_create_info(shader_module->_stage, shader_module->_shader_module, specialization);
}

VkPipelineVertexInputStateCreateInfo lvk::vertex_input_state_create_info() {
	return {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		0,
		VK_NULL_HANDLE,
		0,
		VK_NULL_HANDLE
	};
}
