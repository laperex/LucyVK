#include "lucyvk/vk_init.h"
#include "util/logger.h"
#include <SDL_vulkan.h>
#include <cassert>
#include <iostream>
#include <ostream>
#include <set>
#include <unordered_set>

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
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



lvk_instance lvk::initialize(const char* name, SDL_Window* sdl_window, bool debug_enable) {
	lvk_instance self = {};
	
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
	
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		nullptr,
		0,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		debug_callback,
		nullptr
	};

	if (debug_enable) {
		if (!CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}
		
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
		
		self.layers.push_back("VK_LAYER_KHRONOS_validation");

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

	createInfo.enabledLayerCount = std::size(self.layers);
	createInfo.ppEnabledLayerNames = self.layers.data();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionArray.size());
	createInfo.ppEnabledExtensionNames = requiredExtensionArray.data();

	if (vkCreateInstance(&createInfo, nullptr, &self._instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
	dloggln("Instance Created");

	if (debug_enable) {
		if (CreateDebugUtilsMessengerEXT(self._instance, &debugCreateInfo, nullptr, &self._debug_messenger) != VK_SUCCESS) {
			throw std::runtime_error("debug messenger creation failed!");
		}
		dloggln("Debug Messenger Created");
	}

	if (SDL_Vulkan_CreateSurface(sdl_window, self._instance, &self._surface)) {
		dloggln("Surface Created");
	}

	return self;
}

lvk_instance::~lvk_instance()
{
	if (_debug_messenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, VK_NULL_HANDLE);
		dloggln("DebugUtilMessenger Destroyed");
	}
	
	vkDestroySurfaceKHR(_instance, _surface, VK_NULL_HANDLE);
	dloggln("SurfaceKHR Destroyed");

    vkDestroyInstance(_instance, VK_NULL_HANDLE);
	dloggln("Instance Destroyed");
}

bool lvk_instance::is_debug_enable() {
	return (_debug_messenger != VK_NULL_HANDLE);
}

// ###################################################
// ################# PHYSICAL DEVICE #################
// ###################################################

static lvk::swapchain_support_details QuerySwapchainSupportDetails(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
	lvk::swapchain_support_details details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, VK_NULL_HANDLE);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, VK_NULL_HANDLE);

	if (presentModeCount != 0) {
		details.present_modes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, details.present_modes.data());
	}
	return details;
}

static lvk::queue_family_indices QueryQueueFamilyIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR _surfaceKHR) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, VK_NULL_HANDLE);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	lvk::queue_family_indices indices;

	for (int i = 0; i < queueFamilies.size(); i++) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surfaceKHR, &presentSupport);

		if (queueFamilies[i].queueCount > 0 && presentSupport) {
			indices.present = i;
		}
		
		// if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
		// 	indices.compute = i;
		// }

		// if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
		// 	indices.sparse_binding = i;
		// }

		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics = i;
		}

		if (indices) {
			break;
		}
	}
	
	return indices;
}

static VkPhysicalDevice DefaultPhysicalDeviceSelection(const std::vector<VkPhysicalDevice>& physicalDeviceArray, const lvk_instance* instance) {
	for (const auto& physicalDevice: physicalDeviceArray) {
		bool isRequiredDeviceExtensionsAvailable = false;
		bool isIndicesComplete = false;
		bool isSwapchainAdequate = false;

		{
			uint32_t availableExtensionCount;
			vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &availableExtensionCount, VK_NULL_HANDLE);
			std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
			vkEnumerateDeviceExtensionProperties(physicalDevice, VK_NULL_HANDLE, &availableExtensionCount, availableExtensions.data());

			for (const auto& extension: availableExtensions) {
				if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
					isRequiredDeviceExtensionsAvailable = true;
					break;
				}
			}
		}

		{
			isIndicesComplete = QueryQueueFamilyIndices(physicalDevice, instance->_surface);

			if (isRequiredDeviceExtensionsAvailable) {
				lvk::swapchain_support_details swapchainSupport = QuerySwapchainSupportDetails(physicalDevice, instance->_surface);
				isSwapchainAdequate = !swapchainSupport.formats.empty() && !swapchainSupport.present_modes.empty();
			}

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(physicalDevice, &supportedFeatures);

			if (isIndicesComplete && isRequiredDeviceExtensionsAvailable && isSwapchainAdequate && supportedFeatures.samplerAnisotropy) {
				return physicalDevice;
			}
		}
	}

	return VK_NULL_HANDLE;
}

lvk_physical_device lvk_instance::init_physical_device(lvk::SelectPhysicalDeviceFunction function) {
	lvk_physical_device self = {};
	
	self.instance = this;

	uint32_t availableDeviceCount = 0;

	vkEnumeratePhysicalDevices(_instance, &availableDeviceCount, VK_NULL_HANDLE);
	std::vector<VkPhysicalDevice> availableDevices(availableDeviceCount);
	vkEnumeratePhysicalDevices(_instance, &availableDeviceCount, availableDevices.data());

	self._physical_device = (function == nullptr) ?
		DefaultPhysicalDeviceSelection(availableDevices, this):
		function(availableDevices, this);
	
	if (self._physical_device == nullptr) {
		throw std::runtime_error("failed to find suitable PhysicalDevice!");
	}

	self._queue_family_indices = QueryQueueFamilyIndices(self._physical_device, _surface);
	self._swapchain_support_details = QuerySwapchainSupportDetails(self._physical_device, _surface);

	vkGetPhysicalDeviceFeatures(self._physical_device, &self._features);
	vkGetPhysicalDeviceProperties(self._physical_device, &self._properties);
	
	dloggln("Physical Device - ", self._properties.deviceName);

	return self;
}


lvk_device lvk_physical_device::init_device() {
	lvk_device self = {};
	
	self.instance = this->instance;
	self.physical_device = this;
	
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfoArray;
    std::set<uint32_t> uniqueQueueFamilies = { _queue_family_indices.graphics.value(), _queue_family_indices.present.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily: uniqueQueueFamilies) {
		queueCreateInfoArray.push_back({
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			queueFamily,
			1,
			&queuePriority
		});
    }

	VkDeviceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(std::size(queueCreateInfoArray)),
		queueCreateInfoArray.data(),
		static_cast<uint32_t>(std::size(instance->layers)),
		instance->layers.data(),
		static_cast<uint32_t>(std::size(self.extensions)),
		self.extensions.data(),
		&_features
	};

    if (vkCreateDevice(_physical_device, &createInfo, nullptr, &self._device) != VK_SUCCESS) {
        throw std::runtime_error("logical device creation failed!");
    }
	dloggln("Logical Device Created");

    vkGetDeviceQueue(self._device, _queue_family_indices.graphics.value(), 0, &self._graphicsQueue);
	dloggln("Graphics Queue Created");
    vkGetDeviceQueue(self._device, _queue_family_indices.present.value(), 0, &self._presentQueue);
	dloggln("Present Queue Created");
	
	return self;
}

lvk_device::~lvk_device()
{
	// vkDestroyDevice(_device, VK_NULL_HANDLE);
	// dloggln("Device Destroyed");
}
