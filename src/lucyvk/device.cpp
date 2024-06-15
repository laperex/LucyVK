#include "lucyvk/device.h"
#include "lucyvk/instance.h"
#include "lucyvk/functions.h"

#include "lucyio/logger.h"
#include <set>
#include <stdexcept>

#include "lucyvk/memory.h"

// |--------------------------------------------------
// ----------------> DEVICE
// |--------------------------------------------------


lvk_device_ptr lvk_instance::init_device(std::vector<const char*> extensions, std::vector<const char*> layers, lvk::SelectPhysicalDeviceFunction function) {
	return std::make_unique<lvk_device>(this, extensions, layers, function);
}


lvk_device::lvk_device(lvk_instance* _instance, std::vector<const char*> extensions, std::vector<const char*> layers, lvk::SelectPhysicalDeviceFunction function)
	: extensions(extensions), layers(layers), instance(_instance)
{
	{
		uint32_t availableDeviceCount = 0;

		vkEnumeratePhysicalDevices(instance->_instance, &availableDeviceCount, VK_NULL_HANDLE);
		std::vector<VkPhysicalDevice> availableDevices(availableDeviceCount);
		vkEnumeratePhysicalDevices(instance->_instance, &availableDeviceCount, availableDevices.data());

		this->physical_device._physical_device = (function == nullptr) ?
			lvk::default_physical_device(availableDevices, _instance):
			function(availableDevices, _instance);
		
		if (this->physical_device._physical_device == nullptr) {
			throw std::runtime_error("failed to find suitable PhysicalDevice!");
		}

		this->physical_device._queue_family_indices = lvk::query_queue_family_indices(this->physical_device._physical_device, instance->_surface);
		this->physical_device._swapchain_support_details = lvk::query_swapchain_support_details(this->physical_device._physical_device, instance->_surface);

		vkGetPhysicalDeviceFeatures(this->physical_device._physical_device, &this->physical_device._features);
		vkGetPhysicalDeviceProperties(this->physical_device._physical_device, &this->physical_device._properties);
		
		dloggln(this->physical_device._physical_device, " Physical Device - ", this->physical_device._properties.deviceName);
	}
	
	
	std::set<uint32_t> unique_queue_indices = {
		this->physical_device._queue_family_indices.graphics.value(),
		this->physical_device._queue_family_indices.present.value(),
		this->physical_device._queue_family_indices.compute.value(),
		this->physical_device._queue_family_indices.transfer.value(),
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
		
		.enabledLayerCount = static_cast<uint32_t>(std::size(this->layers)),
		.ppEnabledLayerNames = this->layers.data(),
		
		.enabledExtensionCount = static_cast<uint32_t>(std::size(this->extensions)),
		.ppEnabledExtensionNames = this->extensions.data(),
		
		.pEnabledFeatures = &this->physical_device._features
	};

    if (vkCreateDevice(this->physical_device._physical_device, &create_info, VK_NULL_HANDLE, &this->_device) != VK_SUCCESS) {
        throw std::runtime_error("logical device creation failed!");
    }
	dloggln("Logical Device Created");

    for (uint32_t index: unique_queue_indices) {
		VkQueue queue;
    	vkGetDeviceQueue(this->_device, index, 0, &queue);

		if (index == this->physical_device._queue_family_indices.graphics.value()) {
			this->_graphics_queue = queue;
			dloggln("Graphics Queue Created");
		}
		if (index == this->physical_device._queue_family_indices.present.value()) {
			this->_present_queue = queue;
			dloggln("Present Queue Created");
		}
		if (index == this->physical_device._queue_family_indices.compute.value()) {
			this->_compute_queue = queue;
			dloggln("Compute Queue Created");
		}
		if (index == this->physical_device._queue_family_indices.transfer.value()) {
			this->_transfer_queue = queue;
			dloggln("Transfer Queue Created");
		}
	}
	
	delete [] queue_create_info_array;
	
	// return device;
}

void lvk_device::wait_idle() const {
	vkDeviceWaitIdle(_device);
}

VkResult lvk_device::submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const VkFence fence, uint64_t timeout) const {
	return vkQueueSubmit(_graphics_queue, 1, submit_info, fence);
}

// VkResult lvk_device::submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const VkFence fence, uint64_t timeout) const {
// 	return vkQueueSubmit(_graphics_queue, 1, submit_info, fence);
// }

VkResult lvk_device::submit2(const VkSubmitInfo2* submit_info2, const uint32_t submit_info2_count, const VkFence fence) const {
	return vkQueueSubmit2(_graphics_queue, submit_info2_count, submit_info2, fence);
}

VkResult lvk_device::present(const VkPresentInfoKHR* present_info) const {
	return vkQueuePresentKHR(_present_queue, present_info);
}

lvk_sampler lvk_device::init_sampler(VkFilter min_filter, VkFilter mag_filter, VkSamplerAddressMode sampler_addres_mode) {
	lvk_sampler sampler = {
		._sampler = VK_NULL_HANDLE
	};

	VkSamplerCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		
		.magFilter = mag_filter,
		.minFilter = min_filter,
		
		.addressModeU = sampler_addres_mode,
		.addressModeV = sampler_addres_mode,
		.addressModeW = sampler_addres_mode,
	};

	return sampler;
}

void lvk_device::destroy() {
	deletion_queue.flush();

	vkDestroyDevice(_device, VK_NULL_HANDLE);
	dloggln("Logical Device Destroyed");
}

lvk_device::~lvk_device() {
	dloggln("-- Device Destructor");
}