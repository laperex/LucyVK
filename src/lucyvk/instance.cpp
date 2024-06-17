#include "lucyvk/instance.h"
#include "lucyvk/device.h"

#include "lucyio/logger.h"
#include "lucyvk/functions.h"

#include <SDL_vulkan.h>
#include <iostream>
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


// |--------------------------------------------------
// ----------------> INSTANCE
// |--------------------------------------------------


lvk_instance::~lvk_instance()
{
	// static int i = 0;
	// dloggln("-- Instance Destructor\n", i++);
}

lvk_instance lvk_instance::init(const lvk::config::instance* config, SDL_Window* sdl_window) {
	lvk_instance instance = {};
	
	VkApplicationInfo appInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		config->name,
		VK_MAKE_VERSION(0, 0, 8),
		config->name,
		VK_MAKE_VERSION(1, 1, 7),
		VK_API_VERSION_1_3
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
	
	instance.layers = config->layers;

	if (config->enable_validation_layers) {
		if (!CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		
		instance.layers.push_back("VK_LAYER_KHRONOS_validation");

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

	createInfo.enabledLayerCount = std::size(instance.layers);
	createInfo.ppEnabledLayerNames = instance.layers.data();

	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionArray.size());
	createInfo.ppEnabledExtensionNames = requiredExtensionArray.data();

	if (vkCreateInstance(&createInfo, nullptr, &instance._instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
	dloggln("Instance Created");

	if (config->enable_validation_layers) {
		if (CreateDebugUtilsMessengerEXT(instance._instance, &debugCreateInfo, nullptr, &instance._debug_messenger) != VK_SUCCESS) {
			throw std::runtime_error("debug messenger creation failed!");
		}
		dloggln("Debug Messenger Created");
	}

	if (SDL_Vulkan_CreateSurface(sdl_window, instance._instance, &instance._surfaceKHR)) {
		dloggln("Surface Created");
	}

	return instance;
}

void lvk_instance::destroy()
{
	if (_debug_messenger != VK_NULL_HANDLE) {
        DestroyDebugUtilsMessengerEXT(_instance, _debug_messenger, VK_NULL_HANDLE);
		dloggln("DebugUtilMessenger Destroyed");
	}
	
	vkDestroySurfaceKHR(_instance, _surfaceKHR, VK_NULL_HANDLE);
	dloggln("SurfaceKHR Destroyed");

    vkDestroyInstance(_instance, VK_NULL_HANDLE);
	dloggln("Instance Destroyed");
}

bool lvk_instance::is_debug_enable() {
	return (_debug_messenger != VK_NULL_HANDLE);
}

lvk_device lvk_instance::create_device(std::vector<const char*> extensions, std::vector<const char*> layers, lvk::SelectPhysicalDevice_F function) {
	lvk_device device = {
		._device = VK_NULL_HANDLE,

		._present_queue = VK_NULL_HANDLE,
		._compute_queue = VK_NULL_HANDLE,
		._transfer_queue = VK_NULL_HANDLE,
		._graphics_queue = VK_NULL_HANDLE,
		
		.extensions = extensions,
		.layers = layers,
		
		
		.physical_device = {},
		.deletion_queue = {},

		._surfaceKHR = _surfaceKHR,
		._instance = _instance,
	};

	{
		uint32_t availableDeviceCount = 0;

		vkEnumeratePhysicalDevices(this->_instance, &availableDeviceCount, VK_NULL_HANDLE);
		std::vector<VkPhysicalDevice> availableDevices(availableDeviceCount);
		vkEnumeratePhysicalDevices(this->_instance, &availableDeviceCount, availableDevices.data());

		device.physical_device._physical_device = (function == nullptr) ?
			lvk::default_physical_device(availableDevices, this):
			function(availableDevices, this);
		
		if (device.physical_device._physical_device == nullptr) {
			throw std::runtime_error("failed to find suitable PhysicalDevice!");
		}

		device.physical_device._queue_family_indices = lvk::query_queue_family_indices(device.physical_device._physical_device, this->_surfaceKHR);
		device.physical_device._swapchain_support_details = lvk::query_swapchain_support_details(device.physical_device._physical_device, this->_surfaceKHR);

		vkGetPhysicalDeviceFeatures(device.physical_device._physical_device, &device.physical_device._features);
		vkGetPhysicalDeviceProperties(device.physical_device._physical_device, &device.physical_device._properties);
		
		dloggln(device.physical_device._physical_device, " Physical Device - ", device.physical_device._properties.deviceName);
	}
	
	
	std::set<uint32_t> unique_queue_indices = {
		device.physical_device._queue_family_indices.graphics.value(),
		device.physical_device._queue_family_indices.present.value(),
		device.physical_device._queue_family_indices.compute.value(),
		device.physical_device._queue_family_indices.transfer.value(),
	};

	VkDeviceQueueCreateInfo* queue_create_info_array = new VkDeviceQueueCreateInfo[unique_queue_indices.size()];

    float priority = 1.0f;
	uint32_t i = 0;
    for (auto index: unique_queue_indices) {
		queue_create_info_array[i++] = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = index,
			.queueCount = static_cast<uint32_t>(unique_queue_indices.size()),
			.pQueuePriorities = &priority
		};
    }
	
	VkPhysicalDeviceDynamicRenderingFeatures dynamic_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
		.dynamicRendering = VK_TRUE,
	};
	
	VkPhysicalDeviceSynchronization2Features sync2_features = {
		.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
		.pNext = &dynamic_features,
		.synchronization2 = VK_TRUE,
	};
	
	VkDeviceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		
		// .pNext = &sync2_features,
		
		.queueCreateInfoCount = static_cast<uint32_t>(unique_queue_indices.size()),
		.pQueueCreateInfos = queue_create_info_array,
		
		.enabledLayerCount = static_cast<uint32_t>(std::size(device.layers)),
		.ppEnabledLayerNames = device.layers.data(),
		
		.enabledExtensionCount = static_cast<uint32_t>(std::size(device.extensions)),
		.ppEnabledExtensionNames = device.extensions.data(),
		
		.pEnabledFeatures = &device.physical_device._features
	};

    if (vkCreateDevice(device.physical_device._physical_device, &create_info, VK_NULL_HANDLE, &device._device) != VK_SUCCESS) {
        throw std::runtime_error("logical device creation failed!");
    }
	dloggln("Logical Device Created");

    for (uint32_t index: unique_queue_indices) {
		VkQueue queue;
    	vkGetDeviceQueue(device._device, index, 0, &queue);

		if (index == device.physical_device._queue_family_indices.graphics.value()) {
			device._graphics_queue = queue;
			dloggln("Graphics Queue Created");
		}
		if (index == device.physical_device._queue_family_indices.present.value()) {
			device._present_queue = queue;
			dloggln("Present Queue Created");
		}
		if (index == device.physical_device._queue_family_indices.compute.value()) {
			device._compute_queue = queue;
			dloggln("Compute Queue Created");
		}
		if (index == device.physical_device._queue_family_indices.transfer.value()) {
			device._transfer_queue = queue;
			dloggln("Transfer Queue Created");
		}
	}
	
	delete [] queue_create_info_array;
	
	return device;
}

