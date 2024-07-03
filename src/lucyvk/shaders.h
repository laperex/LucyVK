#include "lucyvk/handles.h"
#include <map>
#include <vulkan/vulkan_core.h>



// ! |--------------------------------------------------
// ! ----------------> SHADER ( OBSELETE SHOULD BE REMOVED AFTER TESTING )
// ! |--------------------------------------------------

struct lvk_shader_module {
	HANDLE_DEF(VkShaderModule, _shader_module)

	const VkShaderStageFlagBits _stage;
};

class lvk_shader_registry {
	std::map<const char*, lvk_shader_module> shader_storage;

public:
	void add(const char* name, lvk_shader_module);
	void get(const char* name, lvk_shader_module);
};
