#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "lucyvk/vk_static.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_info.h"
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


// |--------------------------------------------------
// ----------------> INSTANCE
// |--------------------------------------------------


lvk_instance lvk_init_instance(const lvk::config::instance* config, SDL_Window* sdl_window) {
	lvk_instance instance = {};
	
	VkApplicationInfo appInfo = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,
		nullptr,
		config->name,
		VK_MAKE_VERSION(0, 0, 8),
		config->name,
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

	if (config->enable_validation_layers) {
		if (!CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
		
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

	if (SDL_Vulkan_CreateSurface(sdl_window, instance._instance, &instance._surface)) {
		dloggln("Surface Created");
	}

	return instance;
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


// |--------------------------------------------------
// ----------------> PHYSICAL DEVICE
// |--------------------------------------------------

lvk_physical_device lvk_instance::init_physical_device(lvk::SelectPhysicalDeviceFunction function) {
	lvk_physical_device physical_device = {};
	
	physical_device.instance = this;

	uint32_t availableDeviceCount = 0;

	vkEnumeratePhysicalDevices(_instance, &availableDeviceCount, VK_NULL_HANDLE);
	std::vector<VkPhysicalDevice> availableDevices(availableDeviceCount);
	vkEnumeratePhysicalDevices(_instance, &availableDeviceCount, availableDevices.data());

	physical_device._physical_device = (function == nullptr) ?
		lvk::default_physical_device(availableDevices, this):
		function(availableDevices, this);
	
	if (physical_device._physical_device == nullptr) {
		throw std::runtime_error("failed to find suitable PhysicalDevice!");
	}

	physical_device._queue_family_indices = lvk::query_queue_family_indices(physical_device._physical_device, _surface);
	physical_device._swapchain_support_details = lvk::query_swapchain_support_details(physical_device._physical_device, _surface);

	vkGetPhysicalDeviceFeatures(physical_device._physical_device, &physical_device._features);
	vkGetPhysicalDeviceProperties(physical_device._physical_device, &physical_device._properties);
	
	dloggln(physical_device._physical_device, " Physical Device - ", physical_device._properties.deviceName);

	return physical_device;
}


// |--------------------------------------------------
// ----------------> DEVICE
// |--------------------------------------------------


lvk_device lvk_physical_device::init_device(std::vector<const char*> layers, std::vector<const char*> extensions) {
	lvk_device device = {
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		extensions,
		layers,
		this,
		this->instance,
		{}
	};
	
	std::set<uint32_t> unique_queue_indices = {
		_queue_family_indices.graphics.value(),
		_queue_family_indices.present.value(),
		_queue_family_indices.compute.value(),
		_queue_family_indices.transfer.value(),
	};

	VkDeviceQueueCreateInfo* queue_create_info_array = new VkDeviceQueueCreateInfo[unique_queue_indices.size()];

    float priority = 1.0f;
	uint32_t i = 0;
    for (uint32_t index: unique_queue_indices) {
		queue_create_info_array[i++] = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = index,
			.queueCount = static_cast<uint32_t>(unique_queue_indices.size()),
			.pQueuePriorities = &priority
		};
    }

	VkDeviceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		nullptr,
		0,
		static_cast<uint32_t>(unique_queue_indices.size()),
		queue_create_info_array,
		static_cast<uint32_t>(std::size(device.layers)),
		device.layers.data(),
		static_cast<uint32_t>(std::size(device.extensions)),
		device.extensions.data(),
		&_features
	};

    if (vkCreateDevice(_physical_device, &createInfo, nullptr, &device._device) != VK_SUCCESS) {
        throw std::runtime_error("logical device creation failed!");
    }
	dloggln("Logical Device Created");

    for (uint32_t index: unique_queue_indices) {
		VkQueue queue;
    	vkGetDeviceQueue(device._device, index, 0, &queue);

		if (index == _queue_family_indices.graphics.value()) {
			device._graphics_queue = queue;
			dloggln("Graphics Queue Created");
		}
		if (index == _queue_family_indices.present.value()) {
			device._present_queue = queue;
			dloggln("Present Queue Created");
		}
		if (index == _queue_family_indices.compute.value()) {
			device._compute_queue = queue;
			dloggln("Compute Queue Created");
		}
		if (index == _queue_family_indices.transfer.value()) {
			device._transfer_queue = queue;
			dloggln("Transfer Queue Created");
		}
	}
	
	delete [] queue_create_info_array;
	
	return device;
}

void lvk_device::wait_idle() const {
	vkDeviceWaitIdle(_device);
}

VkResult lvk_device::submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const lvk_fence* fence, uint64_t timeout) const {
	auto result = vkQueueSubmit(_graphics_queue, 1, submit_info, fence->_fence);

	fence->wait(timeout);
	fence->reset();

	return result;
}

VkResult lvk_device::present(const VkPresentInfoKHR* present_info) const {
	return vkQueuePresentKHR(_present_queue, present_info);
}


lvk_device::~lvk_device()
{
	deletion_queue.flush();

	vkDestroyDevice(_device, VK_NULL_HANDLE);
	dloggln("Logical Device Destroyed");
}


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


lvk_swapchain lvk_device::init_swapchain(uint32_t width, uint32_t height) {
	const auto& capabilities = physical_device->_swapchain_support_details.capabilities;

	lvk_swapchain swapchain = {
		VK_NULL_HANDLE,
		VkExtent2D { width, height },
		lvk::get_swapchain_surface_format(physical_device->_swapchain_support_details.formats),
		VK_PRESENT_MODE_FIFO_KHR,
		0,
		VK_NULL_HANDLE,
		{},
		this,
		physical_device,
		instance
	};

	// TODO: support for more presentMode types
	// * VK_PRESENT_MODE_IMMEDIATE_KHR
	// * VK_PRESENT_MODE_MAILBOX_KHR = Mailbox
	// * VK_PRESENT_MODE_FIFO_KHR = V-sync
	// * VK_PRESENT_MODE_FIFO_RELAXED_KHR
	// * VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR
	// * VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR

	for (const auto& availablePresentMode: physical_device->_swapchain_support_details.present_modes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			swapchain._present_mode = availablePresentMode;
			break;
		}
		if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			swapchain._present_mode = availablePresentMode;
			break;
		}
	}
	
	swapchain.recreate(width, height);

	return swapchain;
}

bool lvk_swapchain::recreate(const uint32_t width, const uint32_t height) {
	if (_image_count) {
		for (int i = 0; i < _image_count; i++) {
			vkDestroyImageView(device->_device, _image_views[i], VK_NULL_HANDLE);
		}
		dloggln("ImageViews Destroyed");
	}

	if (_swapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device->_device, _swapchain, VK_NULL_HANDLE);
		dloggln("Swapchain Destroyed");
	}

	this->_extent.width = width;
	this->_extent.height = height;

	const auto& present_modes = physical_device->_swapchain_support_details.present_modes;
	const auto& capabilities = physical_device->_swapchain_support_details.capabilities;

	VkSwapchainCreateInfoKHR createInfo = {};
	{
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = instance->_surface;

		// TF ???
		createInfo.minImageCount = (capabilities.maxImageCount > 0 && capabilities.minImageCount + 1 > capabilities.maxImageCount) ? capabilities.maxImageCount: capabilities.minImageCount + 1;

		createInfo.imageFormat = this->_surface_format.format;
		createInfo.imageColorSpace = this->_surface_format.colorSpace;
		createInfo.imageExtent = (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) ?
			capabilities.currentExtent:
			VkExtent2D {
				std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, this->_extent.width)),
				std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, this->_extent.height))
			};
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// TODO: better approach
		uint32_t queueFamilyIndices[] = {
			physical_device->_queue_family_indices.graphics.value(),
			physical_device->_queue_family_indices.present.value()
		};

		// TODO: Sharing Mode is always exclusive in lvk_buffer. Therefore only one queue is possible
		if (physical_device->_queue_family_indices.present == physical_device->_queue_family_indices.graphics) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		} else {
			throw std::runtime_error("VK_SHARING_MODE_CONCURRENT is not implemented yet");
			
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = std::size(queueFamilyIndices);
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = this->_present_mode;

		// TODO: user defined clipping state
		createInfo.clipped = VK_TRUE;

		// TODO: remains to be tested
		createInfo.oldSwapchain = VK_NULL_HANDLE;


		if (vkCreateSwapchainKHR(this->device->_device, &createInfo, VK_NULL_HANDLE, &this->_swapchain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swapchain!");
		}
		dloggln("Created Swapchain");
	}
	
	// ImageViews
	
	vkGetSwapchainImagesKHR(this->device->_device, this->_swapchain, &_image_count, nullptr);
	_images = new VkImage[_image_count];
	_image_views = new VkImageView[_image_count];
	vkGetSwapchainImagesKHR(this->device->_device, this->_swapchain, &_image_count, _images);

	for (size_t i = 0; i < _image_count; i++) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		viewInfo.image = _images[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = this->_surface_format.format;

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(this->device->_device, &viewInfo, VK_NULL_HANDLE, &this->_image_views[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
	}
	dloggln("ImageViews Created");

	return true;
}

VkResult lvk_swapchain::acquire_next_image(uint32_t* index, VkSemaphore semaphore, VkFence fence, const uint64_t timeout) {
	return vkAcquireNextImageKHR(device->_device, _swapchain, timeout, semaphore, fence, index);
}

lvk_swapchain::~lvk_swapchain()
{
	vkDestroySwapchainKHR(device->_device, _swapchain, VK_NULL_HANDLE);
	dloggln("Swapchain Destroyed");

	for (int i = 0; i < _image_count; i++) {
		vkDestroyImageView(device->_device, _image_views[i], VK_NULL_HANDLE);
	}
	dloggln("ImageViews Destroyed");
	
	delete [] _image_views;
	delete [] _images;
}


// |--------------------------------------------------
// ----------------> COMMAND POOL
// |--------------------------------------------------


lvk_command_pool lvk_device::init_command_pool() {
	return init_command_pool(physical_device->_queue_family_indices.graphics.value(), VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

lvk_command_pool lvk_device::init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags) {
	lvk_command_pool command_pool = {};
	
	command_pool.device = this;
	command_pool.physical_device = this->physical_device;
	command_pool.instance = this->instance;
	command_pool.deletion_queue = &deletion_queue;
	
	VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    poolInfo.queueFamilyIndex = queue_family_index;
    poolInfo.flags = flags;

    if (vkCreateCommandPool(_device, &poolInfo, VK_NULL_HANDLE, &command_pool._command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
	
	deletion_queue.push([=]{
		vkDestroyCommandPool(_device, command_pool._command_pool, VK_NULL_HANDLE);
		dloggln("Command Pool Destroyed");
	});

	return command_pool;
}


// |--------------------------------------------------
// ----------------> COMMAND BUFFER
// |--------------------------------------------------


lvk_command_buffer lvk_command_pool::init_command_buffer(VkCommandBufferLevel level) {
	lvk_command_buffer command_buffer = {
		VK_NULL_HANDLE,
		this,
		device
	};

	VkCommandBufferAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = _command_pool;
	allocateInfo.level = level;
	allocateInfo.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(device->_device, &allocateInfo, &command_buffer._command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("command buffer allocation failed!");
	}
	dloggln("Command Buffer Allocated: ", &command_buffer._command_buffer);
	
	deletion_queue->push([=]{
		vkFreeCommandBuffers(device->_device, _command_pool, 1, &command_buffer._command_buffer);
		dloggln("Command Buffer Destroyed");
	});
	
	return command_buffer;
}

void lvk_command_buffer::reset(VkCommandBufferResetFlags flags) {
	vkResetCommandBuffer(_command_buffer, flags);
}

void lvk_command_buffer::begin(const VkCommandBufferBeginInfo* beginInfo) {
	vkBeginCommandBuffer(_command_buffer, beginInfo);
}

void lvk_command_buffer::begin(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info) {
	VkCommandBufferBeginInfo cmdBeginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = flags,
		.pInheritanceInfo = inheritance_info
	};

	vkBeginCommandBuffer(_command_buffer, &cmdBeginInfo);
}

void lvk_command_buffer::end() {
	vkEndCommandBuffer(_command_buffer);
}

void lvk_command_buffer::begin_render_pass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents subpass_contents) {
	vkCmdBeginRenderPass(_command_buffer, beginInfo, subpass_contents);
}

void lvk_command_buffer::begin_render_pass(const lvk_framebuffer* framebuffer, const VkClearValue* clear_values, const uint32_t clear_value_count, const VkSubpassContents subpass_contents) {
	VkRenderPassBeginInfo beginInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		VK_NULL_HANDLE,
		framebuffer->render_pass->_render_pass,
		framebuffer->_framebuffer,
		{ { 0, 0 }, framebuffer->_extent },
		clear_value_count,
		clear_values,
	};

	begin_render_pass(&beginInfo, subpass_contents);
}

void lvk_command_buffer::end_render_pass() {
	vkCmdEndRenderPass(_command_buffer);
}


// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


lvk_render_pass lvk_device::init_render_pass() {	
	VkAttachmentDescription color_attachment = {};
	color_attachment.flags = 0;
    color_attachment.format = lvk::get_swapchain_surface_format(physical_device->_swapchain_support_details.formats).format;
	// 1 sample = No MSAA
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// clear attachment when loaded
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// store attachment when renderpass ends
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// no stencil
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	

	VkAttachmentDescription depth_attachment = {};
    // Depth attachment
    depth_attachment.flags = 0;
    depth_attachment.format = VK_FORMAT_D32_SFLOAT;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	// subpassDesc.inputAttachmentCount;
	// subpassDesc.pInputAttachments;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	// subpassDesc.pResolveAttachments;
	// subpassDesc.pDepthStencilAttachment;
	// subpassDesc.preserveAttachmentCount;
	// subpassDesc.pPreserveAttachments;
	
	// VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT -> VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT -> VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT

	VkSubpassDependency color_dependency = {};
	color_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	color_dependency.srcAccessMask = 0;
	color_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	color_dependency.dstSubpass = 0;
	color_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;	// | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	color_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depth_dependency = {};
	depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depth_dependency.dstSubpass = 0;
	depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.srcAccessMask = 0;
	depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[2] = { color_attachment, depth_attachment };
	VkSubpassDependency dependency[2] = { color_dependency, depth_dependency };

	return init_render_pass(attachments, 2, &subpass, 1, dependency, 1);
}

lvk_render_pass lvk_device::init_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform) {
	lvk_render_pass render_pass = {
		VK_NULL_HANDLE,
		this,
		physical_device,
		instance,
	};

	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = (enable_transform) * VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM;
	createInfo.attachmentCount = attachment_count;
	createInfo.pAttachments = attachment;
	createInfo.subpassCount = subpass_count;
	createInfo.pSubpasses = subpass;
	createInfo.dependencyCount = dependency_count;
	createInfo.pDependencies = dependency;

	if (vkCreateRenderPass(_device, &createInfo, VK_NULL_HANDLE, &render_pass._render_pass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create renderpass!");
	}
	dloggln("RenderPass Created");

	return render_pass;
}

lvk_render_pass::~lvk_render_pass()
{
	deletion_queue.flush();

	vkDestroyRenderPass(device->_device, _render_pass, VK_NULL_HANDLE);
	dloggln("RenderPass Destroyed");
}


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


lvk_framebuffer lvk_render_pass::init_framebuffer(const VkExtent2D extent, const VkImageView* image_views, const uint32_t image_views_count) {
	lvk_framebuffer framebuffer = {
		VK_NULL_HANDLE,
		extent,
		this,
		device
	};

	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.flags = 0;
	createInfo.renderPass = _render_pass;
	createInfo.width = extent.width;
	createInfo.height = extent.height;
	createInfo.attachmentCount = image_views_count;
	createInfo.pAttachments = image_views;
	createInfo.layers = 1;

	if (vkCreateFramebuffer(device->_device, &createInfo, VK_NULL_HANDLE, &framebuffer._framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffer");
	}
	dloggln("Framebuffer Created");
	
	deletion_queue.push([=]{
		vkDestroyFramebuffer(device->_device, framebuffer._framebuffer, VK_NULL_HANDLE);
		dloggln("Framebuffer Destroyed");
	});

	return framebuffer;
}


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


lvk_semaphore lvk_device::init_semaphore() {
	lvk_semaphore semaphore = {
		VK_NULL_HANDLE,
		this
	};

	VkSemaphoreCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		VK_NULL_HANDLE,
		0
	};
	
	if (vkCreateSemaphore(_device, &createInfo, VK_NULL_HANDLE, &semaphore._semaphore) != VK_SUCCESS) {
		throw std::runtime_error("semaphore creation failed");
	}
	dloggln("Semaphore Created");

	deletion_queue.push([=]{
		vkDestroySemaphore(_device, semaphore._semaphore, VK_NULL_HANDLE);
		dloggln("Semaphore Destroyed");
	});
	
	return semaphore;
}


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


lvk_fence lvk_device::init_fence(VkFenceCreateFlags flags) {
	lvk_fence fence = {
		VK_NULL_HANDLE,
		this
	};

	VkFenceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		VK_NULL_HANDLE,
		flags
	};

	if (vkCreateFence(_device, &createInfo, VK_NULL_HANDLE, &fence._fence) != VK_SUCCESS) {
		throw std::runtime_error("fence creation failed");
	}
	dloggln("Fence Created");
	
	deletion_queue.push([=]{
		vkDestroyFence(_device, fence._fence, VK_NULL_HANDLE);
		dloggln("Fence Destroyed");
	});

	return fence;
}

VkResult lvk_fence::wait(uint64_t timeout) const {
	return vkWaitForFences(device->_device, 1, &_fence, false, timeout);
}

VkResult lvk_fence::reset() const {
	return vkResetFences(device->_device, 1, &_fence);
}


// ! |--------------------------------------------------
// ! ----------------> SHADER ( OBSELETE SHOULD BE REMOVED AFTER TESTING )
// ! |--------------------------------------------------


lvk_shader_module lvk_device::init_shader_module(VkShaderStageFlagBits stage, const char* filename) {
	lvk_shader_module shader_module = {
		VK_NULL_HANDLE,
		stage,
		this
	};
	
	VkShaderModuleCreateInfo info = lvk::info::shader_module(filename);

	assert(info.codeSize);

	if (vkCreateShaderModule(_device, &info, VK_NULL_HANDLE, &shader_module._shader_module) != VK_SUCCESS) {
		throw std::runtime_error(std::string("failed to create shader module! ") + filename);
	}
	dloggln("ShaderModule Created - ", filename);

	return shader_module;
}

lvk_shader_module::~lvk_shader_module()
{
	vkDestroyShaderModule(device->_device, _shader_module, VK_NULL_HANDLE);
	dloggln("Shader Module Destroyed");
}


// |--------------------------------------------------
// ----------------> PIPELINE LAYOUT
// |--------------------------------------------------


lvk_pipeline_layout lvk_device::init_pipeline_layout(const VkPushConstantRange* push_constant_ranges, uint32_t push_constant_range_count, const VkDescriptorSetLayout* descriptor_set_layouts, uint32_t descriptor_set_layout_count) {
	lvk_pipeline_layout pipeline_layout = {
		._pipeline_layout = VK_NULL_HANDLE,
		.device = this,
		.deletion_queue = &deletion_queue,
	};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,

		.setLayoutCount = descriptor_set_layout_count,
		.pSetLayouts = descriptor_set_layouts,

		.pushConstantRangeCount = push_constant_range_count,
		.pPushConstantRanges = push_constant_ranges
	};
	
	if (vkCreatePipelineLayout(this->_device, &pipeline_layout_create_info, VK_NULL_HANDLE, &pipeline_layout._pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("pipeline layout creation failed!");
	}
	dloggln("Pipeline Layout Created");
	
	deletion_queue.push([=]{
		vkDestroyPipelineLayout(_device, pipeline_layout._pipeline_layout, VK_NULL_HANDLE);
		dloggln("Pipeline Layout Destroyed");
	});
	
	return pipeline_layout;
}

lvk_pipeline lvk_pipeline_layout::init_graphics_pipeline(const lvk_render_pass* render_pass, const lvk::config::graphics_pipeline* config) {
	lvk_pipeline pipeline = {
		._pipeline = VK_NULL_HANDLE,
		.pipeline_layout = this,
		.render_pass = render_pass,
		.device = device,
		.type = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.deletion_queue = deletion_queue,
	};

	// TODO: Support for multiple viewport and scissors
	
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;

	viewportState.viewportCount = 1;
	viewportState.pViewports = &config->viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &config->scissor;

	// TODO: Actual Blending to support Transparent Objects
	
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.pNext = nullptr;

	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &config->color_blend_attachment;
	
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;

	pipelineInfo.stageCount = config->shader_stage_array.size();
	pipelineInfo.pStages = config->shader_stage_array.data();
	pipelineInfo.pVertexInputState = &config->vertex_input_state;
	pipelineInfo.pInputAssemblyState = &config->input_assembly_state;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &config->rasterization_state;
	pipelineInfo.pMultisampleState = &config->multisample_state;
	pipelineInfo.pDepthStencilState = &config->depth_stencil_state;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = this->_pipeline_layout;
	pipelineInfo.renderPass = render_pass->_render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	
	if (vkCreateGraphicsPipelines(device->_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("pipeline creation failed!");
	}
	dloggln("Pipeline Created");
	
	deletion_queue->push([=]{
		vkDestroyPipeline(device->_device, pipeline._pipeline, VK_NULL_HANDLE);
		dloggln("Pipeline Destroyed");
	});
	
	return pipeline;
}

lvk_pipeline::~lvk_pipeline()
{
}


// |--------------------------------------------------
// ----------------> ALLOCATOR
// |--------------------------------------------------


lvk_allocator lvk_device::init_allocator() {
	lvk_allocator allocator = {
		VK_NULL_HANDLE,
		this,
		&deletion_queue
	};
	
	VmaAllocatorCreateInfo allocatorInfo = {};

    allocatorInfo.physicalDevice = physical_device->_physical_device;
    allocatorInfo.device = _device;
    allocatorInfo.instance = instance->_instance;

    vmaCreateAllocator(&allocatorInfo, &allocator._allocator);
	dloggln("Allocator Created");
	
	deletion_queue.push([=]{
		vmaDestroyAllocator(allocator._allocator);
		dloggln("Allocator Destroyed");
	});

	return allocator;
}


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


lvk_buffer lvk_allocator::init_buffer(VkBufferUsageFlagBits buffer_usage, VmaMemoryUsage memory_usage, const void* data, const std::size_t size) {
	lvk_buffer buffer = {
		._buffer = VK_NULL_HANDLE,
		._allocation = VK_NULL_HANDLE,
		._allocated_size = size,
		._usage = buffer_usage,
		.allocator = this
	};
	
	VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = buffer_usage,
		
		// TODO: Logical Device Implementation does not support seperate presentation and graphics queue (VK_SHARING_MODE_CONCURRENT) yet
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,

		// .queueFamilyIndexCount = queue_family_indices_count,
		// .pQueueFamilyIndices = queue_family_indices
	};

	VmaAllocationCreateInfo vmaallocInfo = {
		.usage = memory_usage
	};

	//allocate the buffer
	if (vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &buffer._buffer, &buffer._allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	dloggln("Buffer Created: ", lvk::to_string(buffer_usage));
	
	if (data != nullptr) {
		buffer.upload(data, size);
	}

	deletion_queue->push([=]{
		vmaDestroyBuffer(_allocator, buffer._buffer, buffer._allocation);
		dloggln("Buffer Destroyed: ", lvk::to_string(buffer._usage));
	});

	return buffer;
}

lvk_buffer lvk_allocator::init_vertex_buffer(const void* data, const std::size_t size) {
	return init_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, data, size);
}

lvk_buffer lvk_allocator::init_uniform_buffer(const void* data, const std::size_t size) {
	return init_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, data, size);
}

void lvk_buffer::upload(const void* data, const std::size_t size) {
	if (size > _allocated_size) {
		throw std::runtime_error("required size is greater than allocated size!");
	}

	void* _data;
	vmaMapMemory(allocator->_allocator, _allocation, &_data);

	memcpy(_data, data, size);

	vmaUnmapMemory(allocator->_allocator, _allocation);
}


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------


lvk_image lvk_allocator::init_image(VkFormat format, VkImageUsageFlags usage, VkImageType image_type, VkExtent3D extent) {
	lvk_image image = {
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		format,
		image_type,
		extent,
		usage,
		this,
		device,
		deletion_queue
	};
	
	VkImageCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = image_type,
		.format = format,
		.extent = extent,
		
		// TODO: Mipmapping
		.mipLevels = 1,
		
		// TODO: Cubemaps
		.arrayLayers = 1,
		
		// TODO: MSAA
		.samples = VK_SAMPLE_COUNT_1_BIT,

		// * VK_IMAGE_TILING_OPTIMAL					-> Let Vulkan Choose
		// * VK_IMAGE_TILING_LINEAR						-> To read from CPU
		// * VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT	-> Model Specific
		.tiling = VK_IMAGE_TILING_OPTIMAL,

		.usage = usage,
	};

	VmaAllocationCreateInfo allocationInfo = {
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	if (vmaCreateImage(_allocator, &createInfo, &allocationInfo, &image._image, &image._allocation, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("image creation failed!");
	}
	dloggln("Image Created");

	deletion_queue->push([=]{
		vmaDestroyImage(_allocator, image._image, image._allocation);
		dloggln("Image Destroyed");
	});

	return image;
}


// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


lvk_image_view lvk_image::init_image_view(VkImageAspectFlags aspect_flag, VkImageViewType image_view_type) {
	lvk_image_view image_view = {
		._image_view = VK_NULL_HANDLE,
		.image = this
	};

	VkImageViewCreateInfo createInfo = lvk::info::image_view(_image, _format, aspect_flag, image_view_type);

	if (vkCreateImageView(device->_device, &createInfo, VK_NULL_HANDLE, &image_view._image_view) != VK_SUCCESS) {
		throw std::runtime_error("image_view creation failed!");
	}
	dloggln("ImageView Created");

	deletion_queue->push([=]{
		vkDestroyImageView(device->_device, image_view._image_view, VK_NULL_HANDLE);
		dloggln("ImageView Destroyed");
	});

	return image_view;
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


lvk_descriptor_set_layout lvk_device::init_descriptor_set_layout(const VkDescriptorSetLayoutBinding* bindings, const uint32_t binding_count) {
	lvk_descriptor_set_layout descriptor_set_layout = {
		._descriptor_set_layout = VK_NULL_HANDLE,
		.device = this
	};

	VkDescriptorSetLayoutCreateInfo set_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = binding_count,
		.pBindings = bindings
	};

	if (vkCreateDescriptorSetLayout(_device, &set_info, VK_NULL_HANDLE, &descriptor_set_layout._descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("descriptor_set_layout creation failed!");
	}
	dloggln("DescriptorSetLayout Created");

	deletion_queue.push([=]() {
		vkDestroyDescriptorSetLayout(_device, descriptor_set_layout._descriptor_set_layout, VK_NULL_HANDLE);
		dloggln("DescriptorSetLayout Destroyed");
	});

	return descriptor_set_layout;
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR POOL
// |--------------------------------------------------


lvk_descriptor_pool lvk_device::init_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize* descriptor_pool_sizes, const uint32_t descriptor_pool_sizes_count) {
	lvk_descriptor_pool descriptor_pool = {
		._descriptor_pool = VK_NULL_HANDLE,
		.device = this
	};

	VkDescriptorPoolCreateInfo pool_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = max_descriptor_sets,
		.poolSizeCount = descriptor_pool_sizes_count,
		.pPoolSizes = descriptor_pool_sizes
	};

	if (vkCreateDescriptorPool(_device, &pool_info, VK_NULL_HANDLE, &descriptor_pool._descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("descriptor_pool creation failed");
	}
	dloggln("DescriptorPool Created");

	deletion_queue.push([=]() {
		vkDestroyDescriptorPool(_device, descriptor_pool._descriptor_pool, VK_NULL_HANDLE);
		dloggln("DescriptorSetLayout Destroyed");	
	});

	return descriptor_pool;
}


// |--------------------------------------------------
// ----------------> DESCRIPTOR SET
// |--------------------------------------------------


lvk_descriptor_set lvk_descriptor_pool::init_descriptor_set(const lvk_descriptor_set_layout* descriptor_set_layout) {
	lvk_descriptor_set descriptor_set = {
		._descriptor_set = VK_NULL_HANDLE,
		.descriptor_pool = this,
		.device = device
	};

	VkDescriptorSetAllocateInfo allocate_info ={
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = _descriptor_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptor_set_layout->_descriptor_set_layout,
	};

	if (vkAllocateDescriptorSets(device->_device, &allocate_info, &descriptor_set._descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("descriptor_set allocation failed!");
	}
	dloggln("Description Set Allocated");

	return descriptor_set;
}

void lvk_descriptor_set::update(const lvk_buffer* buffer, const std::size_t offset) const {
	VkDescriptorBufferInfo buffer_info = {
		.buffer = buffer->_buffer,
		.offset = offset,
		.range = buffer->_allocated_size
	};
	
	VkWriteDescriptorSet write_set = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = _descriptor_set,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		.pBufferInfo = &buffer_info,
	};

	vkUpdateDescriptorSets(device->_device, 1, &write_set, 0, VK_NULL_HANDLE);
}
