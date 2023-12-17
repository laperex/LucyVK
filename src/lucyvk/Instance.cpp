#include <lucyvk/Instance.h>
#include <lucyvk/PhysicalDevice.h>
#include <lucyvk/LogicalDevice.h>
#include <SDL_vulkan.h>
#include <cstdlib>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <vector>
#include <SDL_vulkan.h>

#include <util/logger.h>
#include <vulkan/vulkan_core.h>

static bool DEBUG_MODE = true;

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks *pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static bool CheckValidationLayerSupport() {
	uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto& layerProperties : availableLayers) {
		if (strcmp("VK_LAYER_KHRONOS_validation", layerProperties.layerName) == 0) {
			return true;
		}
	}

	return false;
}

bool lucyvk::Instance::Initialize(const char* name, SDL_Window* sdl_window) {
	VkApplicationInfo appInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		name,
		VK_MAKE_VERSION(0, 0, 8),
		name,
		VK_MAKE_VERSION(1, 1, 7),
		VK_API_VERSION_1_0
	};
	
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
	
	uint32_t requiredExtensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(sdl_window, &requiredExtensionCount, nullptr);
	std::vector<const char*> requiredExtensionArray(requiredExtensionCount);
	SDL_Vulkan_GetInstanceExtensions(sdl_window, &requiredExtensionCount, requiredExtensionArray.data());
	
	uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensionArray(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensionArray.data());


	if (DEBUG_MODE) {
		if (!CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}
		
		layers.push_back("VK_LAYER_KHRONOS_validation");

		requiredExtensionArray.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		std::unordered_set<std::string> requiredExtensions, availableExtensions;
		
		for (const auto& extension: requiredExtensionArray) {
			requiredExtensions.insert(extension);
		}

		std::vector<const char*> tmp;
		for (const auto& extension: availableExtensionArray) {
			availableExtensions.insert(extension.extensionName);
			tmp.push_back(extension.extensionName);
		}

		for (const auto& extension: requiredExtensions) {
			if (!availableExtensions.contains(extension)) {
				dloggln("Required Extensions: ", requiredExtensionArray);
				dloggln("Available Extensions: ", tmp);
				dloggln(extension);
				throw std::runtime_error("missing required sdl2 extension");
			}
		}
    }

	createInfo.enabledLayerCount = std::size(layers);
	createInfo.ppEnabledLayerNames = layers.data();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionArray.size());
	createInfo.ppEnabledExtensionNames = requiredExtensionArray.data();

	if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
	dloggln("Instance Created");

	if (DEBUG_MODE) {
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			nullptr,
			0,
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
				VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
				VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			DebugCallback,
			nullptr
		};

		if (CreateDebugUtilsMessengerEXT(_instance, &debugCreateInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("DEBUG MESSENGER CREATION FAILED!");
		}
		dloggln("Debug Messenger Created");
	}

	if (SDL_Vulkan_CreateSurface(sdl_window, _instance, &_surface)) {
		dloggln("Surface Created");
	}

	return true;
}

lucyvk::PhysicalDevice lucyvk::Instance::CreatePhysicalDevice() {
	return { *this };
}

lucyvk::LogicalDevice lucyvk::Instance::CreateLogicalDevice(const PhysicalDevice& physicalDevice) {
	return { *this, physicalDevice };
}

// lucyvk::LogicalDevice lucyvk::Instance::CreateLogicalDevice(VkPhysicalDevice physicalDevice) {
// 	// QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

//     // std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
//     // std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

//     // float queuePriority = 1.0f;
//     // for (uint32_t queueFamily : uniqueQueueFamilies) {
//     //     VkDeviceQueueCreateInfo queueCreateInfo = {};
//     //     queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//     //     queueCreateInfo.queueFamilyIndex = queueFamily;
//     //     queueCreateInfo.queueCount = 1;
//     //     queueCreateInfo.pQueuePriorities = &queuePriority;
//     //     queueCreateInfos.push_back(queueCreateInfo);
//     // }

//     // VkPhysicalDeviceFeatures deviceFeatures = {};
//     // deviceFeatures.samplerAnisotropy = VK_TRUE;

//     // VkDeviceCreateInfo createInfo = {};
//     // createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

//     // createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
//     // createInfo.pQueueCreateInfos = queueCreateInfos.data();

//     // createInfo.pEnabledFeatures = &deviceFeatures;
//     // createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
//     // createInfo.ppEnabledExtensionNames = deviceExtensions.data();

// 	// createInfo.enabledLayerCount = 0;

//     // if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device_) != VK_SUCCESS) {
//     //     throw std::runtime_error("failed to create logical device!");
//     // }

//     // vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
//     // vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);

// 	return { *this };
// }
