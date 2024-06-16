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

// void lvk_device::allocate_command_buffers(const VkCommandPool command_pool, VkCommandBufferLevel level, lvk_command_buffer* command_buffers, uint32_t command_buffers_count) {
// 	// lvk_command_buffer command_buffer = {
// 	// 	._command_buffer = VK_NULL_HANDLE,
// 	// };

// 	VkCommandBufferAllocateInfo allocate_info = {
// 		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

// 		.commandPool = command_pool,
// 		.level = level,
// 		.commandBufferCount = 1,
// 	};

// 	if (vkAllocateCommandBuffers(device->_device, &allocate_info, &command_buffer._command_buffer) != VK_SUCCESS) {
// 		throw std::runtime_error("command buffer allocation failed!");
// 	}
// 	dloggln("Command Buffer Allocated: ", &command_buffer._command_buffer);
// }



// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


lvk_semaphore lvk_device::create_semaphore() {
	lvk_semaphore semaphore = {
		._semaphore = VK_NULL_HANDLE
	};
	
	VkSemaphoreCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = 0,
	};

	if (vkCreateSemaphore(this->_device, &create_info, VK_NULL_HANDLE, &semaphore._semaphore) != VK_SUCCESS) {
		throw std::runtime_error("semaphore creation failed");
	}
	dloggln("Semaphore Created");
	
	deletion_queue.push([=]{
		vkDestroySemaphore(this->_device, semaphore._semaphore, VK_NULL_HANDLE);
		dloggln("Semaphore Destroyed");
	});
	
	return semaphore;
}


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


lvk_fence lvk_device::create_fence(VkFenceCreateFlags flags) {
	lvk_fence fence = {
		._fence = VK_NULL_HANDLE
	};
	
	VkFenceCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = VK_NULL_HANDLE,
		.flags = flags
	};

	if (vkCreateFence(this->_device, &create_info, VK_NULL_HANDLE, &fence._fence) != VK_SUCCESS) {
		throw std::runtime_error("fence creation failed");
	}
	dloggln("Fence Created");
	
	deletion_queue.push([=]{
		vkDestroyFence(_device, fence._fence, VK_NULL_HANDLE);
		dloggln("Fence Destroyed");
	});
	
	return fence;
}

void lvk_device::wait_for_fence(const lvk_fence& fence, uint64_t timeout) {
	wait_for_fences(&fence, 1, timeout);
}

void lvk_device::wait_for_fences(const lvk_fence* fence, uint32_t fence_count, uint64_t timeout) {
	vkWaitForFences(this->_device, fence_count, &fence->_fence, true, timeout);
}

void lvk_device::reset_fence(const lvk_fence& fence) {
	reset_fences(&fence, 1);
}

void lvk_device::reset_fences(const lvk_fence* fence, uint32_t fence_count) {
	vkResetFences(this->_device, fence_count, &fence->_fence);
}



// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


lvk_command_pool lvk_device::create_graphics_command_pool() {
	return create_command_pool(physical_device._queue_family_indices.graphics.value(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}


lvk_command_pool lvk_device::create_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
	lvk_command_pool command_pool = {
		._command_pool = VK_NULL_HANDLE
	};

	VkCommandPoolCreateInfo create_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = flags,
		.queueFamilyIndex = queue_family_index,
	};

    if (vkCreateCommandPool(_device, &create_info, VK_NULL_HANDLE, &command_pool._command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
	
	deletion_queue.push([=]{
		vkDestroyCommandPool(_device, command_pool._command_pool, VK_NULL_HANDLE);
		dloggln("Command Pool Destroyed");
	});

	return command_pool;
}

lvk_command_buffer lvk_device::allocate_command_buffer_unique(const lvk_command_pool& command_pool, VkCommandBufferLevel level) {
	lvk_command_buffer command_buffer = {
		._command_buffer = VK_NULL_HANDLE
	};

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

		.commandPool = command_pool._command_pool,
		.level = level,
		.commandBufferCount = 1,
	};

	if (vkAllocateCommandBuffers(this->_device, &allocate_info, &command_buffer._command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("command buffer allocation failed!");
	}
	dloggln("Command Buffer Allocated: ");
	
	
	deletion_queue.push([=]{
		vkFreeCommandBuffers(this->_device, command_pool._command_pool, 1, &command_buffer._command_buffer);
		dloggln("Command Buffer Destroyed");
	});
	
	return command_buffer;
}

std::vector<lvk_command_buffer> lvk_device::allocate_command_buffers(const lvk_command_pool& command_pool, uint32_t command_buffer_count, VkCommandBufferLevel level) {
	const std::vector<lvk_command_buffer> command_buffer_array(command_buffer_count);

	VkCommandBufferAllocateInfo allocate_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,

		.commandPool = command_pool._command_pool,
		.level = level,
		.commandBufferCount = command_buffer_count,
	};

	if (vkAllocateCommandBuffers(this->_device, &allocate_info, (VkCommandBuffer*)command_buffer_array.data()->_command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("command buffers allocation failed!");
	}
	dloggln("Command Buffers Allocated: ");
	
	deletion_queue.push([=]{
		vkFreeCommandBuffers(this->_device, command_pool._command_pool, command_buffer_count, (VkCommandBuffer*)command_buffer_array.data()->_command_buffer);
		dloggln("Command Buffers Destroyed");
	});
	
	return command_buffer_array;
}
