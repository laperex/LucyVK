#include "instance.h"
#include <cstdlib>
#include <stdexcept>
#include <unordered_set>
#include <vector>

#include <util/logger.h>

static bool DEBUG_MODE = true;

// static bool checksdl2requiredextensions() {
// 	uint32_t extensionCount = 0;
//     vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
//     std::vector<VkExtensionProperties> availableExtensionArray(extensionCount);
//     vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensionArray.data());

//     std::unordered_set<const char*> requiredExtensions, availableExtensions;
	
//     for (const auto& extension: GetRequiredExtensions()) {
//         // std::cout << "\t" << extension.extensionName << std::endl;
//         requiredExtensions.insert(extension);
//     }
	
// 	std::cout << "Available Extensions:\n";
//     for (const auto& extension: availableExtensionArray) {
// 		availableExtensions.insert(extension.extensionName);

// 		if (requiredExtensions.contains(extension.extensionName)) {
// 			continue;
// 		}
//         std::cout << '\t' << extension.extensionName << '\n';
//     }
	
// 	bool requiredExtensionsAvailable = true;

// 	std::cout << "Required Extensions:\n";
// 	for (const auto& extension: requiredExtensions) {
// 		std::cout << '\t' << extension;
		
// 		// if (availableExtensions.contains(extension)) {
// 		// 	std::cout << "\t- Unavailable";
// 		// 	// requiredExtensionsAvailable = false;
// 		// }
		
// 		std::cout << '\n';
//     }

// 	return requiredExtensionsAvailable;
// }

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
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

static VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo() {
	return {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		nullptr,
		0,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		debugCallback,
    	nullptr
	};
}

static bool checkValidationLayerSupport() {
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


lucyvk::instance::instance() {
	
}

lucyvk::instance::~instance() {
	
}

bool lucyvk::instance::init(const char* name, SDL_Window* sdl_window) {
	VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = name;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = name;
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
	
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

	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionArray.size());
	createInfo.ppEnabledExtensionNames = requiredExtensionArray.data();

	//
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = DebugMessengerCreateInfo();
	//

	if (DEBUG_MODE) {
		if (!checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		requiredExtensionArray.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		
        createInfo.enabledLayerCount = static_cast<uint32_t>(std::size(layers));
        createInfo.ppEnabledLayerNames = layers;

        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
	
	if (vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

	{
		std::unordered_set<std::string> requiredExtensions, availableExtensions;
		
		for (const auto& extension: requiredExtensionArray) {
			requiredExtensions.insert(extension);
		}

		std::vector<const char*> tmp;
		for (const auto& extension: availableExtensionArray) {
			availableExtensions.insert(extension.extensionName);
			tmp.push_back(extension.extensionName);
		}

		dloggln("Required Extensions: ", requiredExtensionArray);
		dloggln("Available Extensions: ", tmp);

		for (const auto& extension: requiredExtensions) {
			if (!availableExtensions.contains(extension)) {
				dloggln(extension);
				throw std::runtime_error("missing required sdl2 extension");
			}
		}
	}
	
	
	return true;
}
