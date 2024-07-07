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

lvk_instance lvk_instance::initialize(const char* name, SDL_Window* sdl_window, bool enable_validation_layers, std::vector<const char*> layers, std::vector<const char*> extensions) {
	lvk_instance instance = {
		._instance = VK_NULL_HANDLE,
		._surfaceKHR = VK_NULL_HANDLE,
		._debug_messenger = VK_NULL_HANDLE,
	};
	
	VkApplicationInfo app_info = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		
		.pApplicationName = name,
		.applicationVersion = VK_MAKE_VERSION(1, 0, 1),
		
		.pEngineName = name,
		.engineVersion = VK_MAKE_VERSION(1, 1, 7),
		
		.apiVersion = VK_API_VERSION_1_3
	};

	uint32_t requiredExtensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(sdl_window, &requiredExtensionCount, nullptr);
	std::vector<const char*> requiredExtensionArray(requiredExtensionCount);
	SDL_Vulkan_GetInstanceExtensions(sdl_window, &requiredExtensionCount, requiredExtensionArray.data());
	
	uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensionArray(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensionArray.data());
	
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
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
	
	// instance.layers = config->layers;
	
	VkInstanceCreateInfo create_info = {
    	.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    	.pApplicationInfo = &app_info,
	};

	if (enable_validation_layers) {
		if (!CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
		
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

	create_info.enabledLayerCount = std::size(layers);
	create_info.ppEnabledLayerNames = layers.data();

	create_info.enabledExtensionCount = static_cast<uint32_t>(requiredExtensionArray.size());
	create_info.ppEnabledExtensionNames = requiredExtensionArray.data();

	if (vkCreateInstance(&create_info, VK_NULL_HANDLE, &instance._instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
	dloggln("Instance Created");

	if (enable_validation_layers) {
		if (CreateDebugUtilsMessengerEXT(instance, &debug_create_info, VK_NULL_HANDLE, &instance._debug_messenger) != VK_SUCCESS) {
			throw std::runtime_error("debug messenger creation failed!");
		}
		dloggln("Debug Messenger Created");
	}

	if (SDL_Vulkan_CreateSurface(sdl_window, instance, &instance._surfaceKHR)) {
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

lvk_device lvk_instance::create_device(std::vector<const char*> extensions, lvk::SelectPhysicalDevice_F function) {
	lvk_device device = {
		._device = VK_NULL_HANDLE,
		._allocator = VK_NULL_HANDLE,

		.extensions = extensions,

		._queue = {
			.graphics = {
				.handle = VK_NULL_HANDLE
			},
			.present = {
				.handle = VK_NULL_HANDLE
			},
			.compute = {
				.handle = VK_NULL_HANDLE
			},
			.transfer = {
				.handle = VK_NULL_HANDLE
			},
		},

		.physical_device = {
			._physical_device = VK_NULL_HANDLE,
		},

		.destroyer = {},

		._surfaceKHR = _surfaceKHR,
	};

	{
		uint32_t available_device_count = 0;
		vkEnumeratePhysicalDevices(this->_instance, &available_device_count, VK_NULL_HANDLE);
		std::vector<VkPhysicalDevice> available_devices(available_device_count);
		vkEnumeratePhysicalDevices(this->_instance, &available_device_count, available_devices.data());

		device.physical_device._physical_device = (function == nullptr) ?
			lvk::default_physical_device(available_devices, this):
			function(available_devices, this);
		
		if (device.physical_device._physical_device == nullptr) {
			throw std::runtime_error("failed to find suitable PhysicalDevice!");
		}

		// device._queue_family_indices = lvk::query_queue_family_indices(device.physical_device._physical_device, this->_surfaceKHR);
		// queue indices initialization
		{
			uint32_t queue_family_count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device.physical_device, &queue_family_count, VK_NULL_HANDLE);
			std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
			vkGetPhysicalDeviceQueueFamilyProperties(device.physical_device, &queue_family_count, queue_families.data());
			
			for (int i = 0; i < queue_families.size() && device._queue == false; i++) {
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device.physical_device, i, _surfaceKHR, &presentSupport);

				if (queue_families[i].queueCount > 0 && presentSupport) {
					device._queue.present.index = i;
				}
				
				if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
					device._queue.compute.index = i;
				}

				if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
					device._queue.transfer.index = i;
				}

				if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					device._queue.graphics.index = i;
				}
			}
		}
		
		// device._swapchain_support_details = lvk::query_swapchain_support_details(device.physical_device._physical_device, this->_surfaceKHR);
		{
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.physical_device, _surfaceKHR, &device._swapchain_support_details.capabilities);
			uint32_t format_count;
			vkGetPhysicalDeviceSurfaceFormatsKHR(device.physical_device, _surfaceKHR, &format_count, VK_NULL_HANDLE);

			if (format_count != 0) {
				device._swapchain_support_details.formats.resize(format_count);
				vkGetPhysicalDeviceSurfaceFormatsKHR(device.physical_device, _surfaceKHR, &format_count, device._swapchain_support_details.formats.data());
			}

			uint32_t present_mode_count;
			vkGetPhysicalDeviceSurfacePresentModesKHR(device.physical_device, _surfaceKHR, &present_mode_count, VK_NULL_HANDLE);

			if (present_mode_count != 0) {
				device._swapchain_support_details.present_modes.resize(present_mode_count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(device.physical_device, _surfaceKHR, &present_mode_count, device._swapchain_support_details.present_modes.data());
			}
		}

		vkGetPhysicalDeviceFeatures(device.physical_device._physical_device, &device.physical_device._features);
		vkGetPhysicalDeviceProperties(device.physical_device._physical_device, &device.physical_device._properties);
		
		dloggln("Created:\t", device.physical_device._physical_device, "\t [Physical Device] - ", device.physical_device._properties.deviceName);
	}
	
	
	std::set<uint32_t> unique_queue_indices = {
		device._queue.graphics.index.value(),
		device._queue.present.index.value(),
		device._queue.compute.index.value(),
		device._queue.transfer.index.value(),
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
		
		.enabledExtensionCount = static_cast<uint32_t>(std::size(device.extensions)),
		.ppEnabledExtensionNames = device.extensions.data(),
		
		.pEnabledFeatures = &device.physical_device._features
	};

    if (vkCreateDevice(device.physical_device._physical_device, &create_info, VK_NULL_HANDLE, &device._device) != VK_SUCCESS) {
        throw std::runtime_error("logical device creation failed!");
    }
	dloggln("Created:\t", device._device, "\t [LogicalDevice]");


    for (uint32_t index: unique_queue_indices) {
		VkQueue queue;
    	vkGetDeviceQueue(device._device, index, 0, &queue);

		if (index == device._queue.graphics.index.value()) {
			device._queue.graphics.handle = queue;
			dloggln("Created:\t", queue, "\t [Graphics Queue]");
		}
		if (index == device._queue.present.index.value()) {
			device._queue.present.handle = queue;
			dloggln("Created:\t", queue, "\t [Present Queue]");
		}
		if (index == device._queue.compute.index.value()) {
			device._queue.compute.handle = queue;
			dloggln("Created:\t", queue, "\t [Compute Queue]");
		}
		if (index == device._queue.transfer.index.value()) {
			device._queue.transfer.handle = queue;
			dloggln("Created:\t", queue, "\t [Transfer Queue]");
		}
	}
	
	delete [] queue_create_info_array;
	
	
	VmaAllocatorCreateInfo allocator_create_info = {
		.physicalDevice = device.physical_device,
		.device = device,
		.instance = _instance,
	};

    vmaCreateAllocator(&allocator_create_info, &device._allocator);
	dloggln("Created:\t", device._allocator, "\t [Allocator]");
	
	return device;
}

