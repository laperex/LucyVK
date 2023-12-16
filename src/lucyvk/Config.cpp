#include <vector>
#include <vulkan/vulkan_core.h>

namespace lucyvk {
	struct {
		struct {
			std::vector<const char*> layers = { "VK_LAYER_KHRONOS_validation" };
			std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		} Instance;
		
		struct {
			
		} PhysicalDevice;
	} config;
}