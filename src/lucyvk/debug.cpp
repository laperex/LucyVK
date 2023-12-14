// #include "debug.h"
// #include <iostream>
// #include <vulkan/vulkan.hpp>

// static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
//     VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
//     std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

//     return VK_FALSE;
// }

// static VkResult CreateDebugUtilsMessengerEXT(
//     VkInstance instance,
//     const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
//     const VkAllocationCallbacks *pAllocator,
//     VkDebugUtilsMessengerEXT *pDebugMessenger) {
//     auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
//     if (func != nullptr) {
//         return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
//     } else {
//         return VK_ERROR_EXTENSION_NOT_PRESENT;
//     }
// }

// static void DestroyDebugUtilsMessengerEXT(
//     VkInstance instance,
//     VkDebugUtilsMessengerEXT debugMessenger,
//     const VkAllocationCallbacks *pAllocator) {
//     auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
//     if (func != nullptr) {
//         func(instance, debugMessenger, pAllocator);
//     }
// }

// static VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo() {
// 	return {
// 		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
// 		nullptr,
// 		0,
// 		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
// 			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
// 		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
// 			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
// 			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
// 		debugCallback,
//     	nullptr
// 	};
// }

// bool lucyvk::debugger::create() {
// 	// if (!LUCY_VK_ENABLE_VALIDATION_LAYERS) return;

// 	VkDebugUtilsMessengerCreateInfoEXT createInfo = DebugMessengerCreateInfo();

// 	if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
// 		throw std::runtime_error("DEBUG MESSENGER CREATION FAILED!");
// 	}
// }