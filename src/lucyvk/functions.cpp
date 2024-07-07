// #include "lucyvk/physical_device.h"
#include "lucyvk/functions.h"
#include "lucyvk/instance.h"
#include "lucyvk/device.h"
#include "lucyio/logger.h"
#include <vulkan/vulkan_core.h>


// VkSurfaceFormatKHR lvk::get_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& format_array) {
// 	for (const auto& availableFormat: format_array) {
// 		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
// 			return availableFormat;
// 		}
// 	}
	
// 	return format_array[0];
// }

// lvk::swapchain_support_details lvk::query_swapchain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
// 	lvk::swapchain_support_details details;
// 	// vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);
// 	// uint32_t formatCount;
// 	// vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, VK_NULL_HANDLE);

// 	// if (formatCount != 0) {
// 	// 	details.formats.resize(formatCount);
// 	// 	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &formatCount, details.formats.data());
// 	// }

// 	// uint32_t presentModeCount;
// 	// vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, VK_NULL_HANDLE);

// 	// if (presentModeCount != 0) {
// 	// 	details.present_modes.resize(presentModeCount);
// 	// 	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &presentModeCount, details.present_modes.data());
// 	// }
// 	return details;
// }

VkPhysicalDevice lvk::default_physical_device(const std::vector<VkPhysicalDevice>& physicalDeviceArray, const lvk_instance* instance) {
	dloggln("");
	dloggln("Available GPUs:");
	
	
	VkPhysicalDeviceProperties properties;
	for (const auto& physical_device: physicalDeviceArray) {
		vkGetPhysicalDeviceProperties(physical_device, &properties);
		
		dloggln("\t- ", properties.deviceName);

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, instance->_surfaceKHR, &present_mode_count, VK_NULL_HANDLE);
		std::vector<VkPresentModeKHR> present_modes(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, instance->_surfaceKHR, &present_mode_count, present_modes.data());

		for (const auto& mode: present_modes) {
			dloggln("\t\t- ", to_string(mode));
		}
	}

	for (const auto& physical_device: physicalDeviceArray) {
		static int i = 0;
		
		if (i++ == 0) continue;

		bool isRequiredDeviceExtensionsAvailable = false;
		bool isIndicesComplete = false;
		bool isSwapchainAdequate = false;

		{
			uint32_t availableExtensionCount;
			vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE, &availableExtensionCount, VK_NULL_HANDLE);
			std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
			vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE, &availableExtensionCount, availableExtensions.data());

			for (const auto& extension: availableExtensions) {
				if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
					isRequiredDeviceExtensionsAvailable = true;
					break;
				}
			}
		}

		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> present_modes;
			
			struct {
				std::optional<uint32_t> graphics;
				std::optional<uint32_t> present;
				std::optional<uint32_t> compute;
				std::optional<uint32_t> transfer;

				operator bool() const {
					return graphics.has_value() && present.has_value() && compute.has_value() && transfer.has_value();
				}
			} indices;

			if (isRequiredDeviceExtensionsAvailable) {
				vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, instance->_surfaceKHR, &capabilities);
				uint32_t format_count;
				vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, instance->_surfaceKHR, &format_count, VK_NULL_HANDLE);

				if (format_count != 0) {
					formats.resize(format_count);
					vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, instance->_surfaceKHR, &format_count, formats.data());
				}

				uint32_t present_mode_count;
				vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, instance->_surfaceKHR, &present_mode_count, VK_NULL_HANDLE);

				if (present_mode_count != 0) {
					present_modes.resize(present_mode_count);
					vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, instance->_surfaceKHR, &present_mode_count, present_modes.data());
				}

				isSwapchainAdequate = !formats.empty() && !present_modes.empty();
			}

			VkPhysicalDeviceFeatures supportedFeatures;
			vkGetPhysicalDeviceFeatures(physical_device, &supportedFeatures);

			if (isRequiredDeviceExtensionsAvailable && isSwapchainAdequate && supportedFeatures.samplerAnisotropy) {
				return physical_device;
			}
		}
	}

	return VK_NULL_HANDLE;
}

const char* lvk::to_string(const VkPresentModeKHR mode) {
	switch (mode) {
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			return "VK_PRESENT_MODE_IMMEDIATE_KHR";
		case VK_PRESENT_MODE_MAILBOX_KHR:
			return "VK_PRESENT_MODE_MAILBOX_KHR";
		case VK_PRESENT_MODE_FIFO_KHR:
			return "VK_PRESENT_MODE_FIFO_KHR";
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
			return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
			return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
		case VK_PRESENT_MODE_MAX_ENUM_KHR:
			return "VK_PRESENT_MODE_MAX_ENUM_KHR";
	}
}

const char* lvk::to_string(const VkBufferUsageFlagBits usage) {
	switch (usage) {
        case VK_BUFFER_USAGE_TRANSFER_SRC_BIT:
			return "TRANSFER_SRC";
        case VK_BUFFER_USAGE_TRANSFER_DST_BIT:
			return "TRANSFER_DST";
        case VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT:
			return "UNIFORM_TEXEL_BUFFER";
        case VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT:
			return "STORAGE_TEXEL_BUFFER";
        case VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT:
			return "UNIFORM_BUFFER";
        case VK_BUFFER_USAGE_STORAGE_BUFFER_BIT:
			return "STORAGE_BUFFER";
        case VK_BUFFER_USAGE_INDEX_BUFFER_BIT:
			return "INDEX_BUFFER";
        case VK_BUFFER_USAGE_VERTEX_BUFFER_BIT:
			return "VERTEX_BUFFER";
        case VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT:
			return "INDIRECT_BUFFER";
        case VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT:
			return "SHADER_DEVICE_ADDRESS";
        case VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR:
			return "VIDEO_DECODE_SRC";
        case VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR:
			return "VIDEO_DECODE_DST";
        case VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_BIT_EXT:
			return "TRANSFORM_FEEDBACK_BUFFER";
        case VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT:
			return "TRANSFORM_FEEDBACK_COUNTER_BUFFER";
        case VK_BUFFER_USAGE_CONDITIONAL_RENDERING_BIT_EXT:
			return "CONDITIONAL_RENDERING";
        case VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR:
			return "ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY";
        case VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR:
			return "ACCELERATION_STRUCTURE_STORAGE";
        case VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR:
			return "SHADER_BINDING_TABLE";
        case VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT:
			return "SAMPLER_DESCRIPTOR_BUFFER";
        case VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT:
			return "RESOURCE_DESCRIPTOR_BUFFER";
        case VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT:
			return "PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER";
        case VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT:
			return "MICROMAP_BUILD_INPUT_READ_ONLY";
        case VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT:
			return "MICROMAP_STORAGE";
        case VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR:
			return "VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR";
        case VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR:
			return "VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR";
        case VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM:
			return "VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM";
          break;
        }

        return "";
}

// VkPipelineColorBlendAttachmentState lvk::color_blend_attachment() {
// 	return {
// 		.blendEnable = VK_FALSE,
// 		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
// 	};
// }

// VkDescriptorSetLayoutBinding lvk::descriptor_set_layout_binding(uint32_t binding, VkShaderStageFlags shader_stage_flags, VkDescriptorType descriptor_type, uint32_t descriptor_count) {
// 	return {
// 		.binding = binding,
// 		.descriptorType = descriptor_type,
// 		.descriptorCount = descriptor_count,
// 		.stageFlags = shader_stage_flags,
// 	};
// }

// VkImageSubresourceRange lvk::image_subresource_range(VkImageAspectFlags aspect_mask) {
// 	return {
// 		.aspectMask = aspect_mask,
// 		.baseMipLevel = 0,
// 		.levelCount = VK_REMAINING_MIP_LEVELS,
// 		.baseArrayLayer = 0,
// 		.layerCount = VK_REMAINING_ARRAY_LAYERS,
// 	};
// }

