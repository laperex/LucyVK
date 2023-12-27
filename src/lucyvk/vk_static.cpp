#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "lucyvk/vk_static.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_pipeline.h"
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
		extensions,
		layers,
		this,
		this->instance
	};
	
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

    vkGetDeviceQueue(device._device, _queue_family_indices.graphics.value(), 0, &device._graphicsQueue);
	dloggln("Graphics Queue Created");
    vkGetDeviceQueue(device._device, _queue_family_indices.present.value(), 0, &device._presentQueue);
	dloggln("Present Queue Created");
	
	return device;
}

void lvk_device::wait_idle() const {
	vkDeviceWaitIdle(_device);
}

lvk_device::~lvk_device()
{
	vkDestroyDevice(_device, VK_NULL_HANDLE);
	dloggln("Device Destroyed");
}


// |--------------------------------------------------
// ----------------> SWAPCHAIN
// |--------------------------------------------------


static VkSurfaceFormatKHR get_swapchain_surface_format(const std::vector<VkSurfaceFormatKHR>& format_array) {
	for (const auto& availableFormat: format_array) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	
	return format_array[0];
}


lvk_swapchain lvk_device::init_swapchain(uint32_t width, uint32_t height) {
	const auto& capabilities = physical_device->_swapchain_support_details.capabilities;

	lvk_swapchain swapchain = {
		VK_NULL_HANDLE,
		VkExtent2D { width, height },
		get_swapchain_surface_format(physical_device->_swapchain_support_details.formats),
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
		// if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
		// 	self->_present_mode = availablePresentMode;
		// 	break;
		// }
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

		if (physical_device->_queue_family_indices.unique()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		} else {
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
	lvk_command_pool self = {};
	
	self.device = this;
	self.physical_device = this->physical_device;
	self.instance = this->instance;
	
	VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    poolInfo.queueFamilyIndex = queue_family_index;
    poolInfo.flags = flags;

    if (vkCreateCommandPool(_device, &poolInfo, VK_NULL_HANDLE, &self._command_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

	return self;
}

lvk_command_pool::~lvk_command_pool()
{
	vkDestroyCommandPool(device->_device, _command_pool, VK_NULL_HANDLE);
	dloggln("Command Pool Destroyed");
}


// |--------------------------------------------------
// ----------------> COMMAND BUFFER
// |--------------------------------------------------


lvk_command_buffer lvk_command_pool::init_command_buffer(VkCommandBufferLevel level) {
	lvk_command_buffer self = {
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

	if (vkAllocateCommandBuffers(device->_device, &allocateInfo, &self._command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("command buffer allocation failed!");
	}
	dloggln("Command Buffer Allocated: ", &self._command_buffer);
	
	return self;
}

void lvk_command_buffer::reset(VkCommandBufferResetFlags flags) {
	
	vkResetCommandBuffer(_command_buffer, flags);
}

void lvk_command_buffer::begin(const VkCommandBufferBeginInfo* beginInfo) {
	vkBeginCommandBuffer(_command_buffer, beginInfo);
}

void lvk_command_buffer::begin(const VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo* inheritance_info) {
	VkCommandBufferBeginInfo beginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		VK_NULL_HANDLE,
		flags,
		inheritance_info
	};

	vkBeginCommandBuffer(_command_buffer, &beginInfo);
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

	vkCmdBeginRenderPass(_command_buffer, &beginInfo, subpass_contents);
}

void lvk_command_buffer::end_render_pass() {
	vkCmdEndRenderPass(_command_buffer);
}

lvk_command_buffer::~lvk_command_buffer()
{
	vkFreeCommandBuffers(device->_device, command_pool->_command_pool, 1, &_command_buffer);
	dloggln("Command Buffer Destroyed");
}


// |--------------------------------------------------
// ----------------> RENDER PASS
// |--------------------------------------------------


lvk_render_pass lvk_device::init_render_pass() {	
	VkAttachmentDescription attachment = {};
	attachment.flags = 0;
    attachment.format = get_swapchain_surface_format(physical_device->_swapchain_support_details.formats).format;
	// 1 sample = No MSAA
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// clear attachment when loaded
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// store attachment when renderpass ends
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// no stencil
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference attachment_reference = {};
	attachment_reference.attachment = 0;
    attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.flags = 0;
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	// subpassDesc.inputAttachmentCount;
	// subpassDesc.pInputAttachments;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachment_reference;
	// subpassDesc.pResolveAttachments;
	// subpassDesc.pDepthStencilAttachment;
	// subpassDesc.preserveAttachmentCount;
	// subpassDesc.pPreserveAttachments;

	// VkSubpassDependency dependency = {};
	// dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	// dependency.srcAccessMask = 0;
	// dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	// dependency.dstSubpass = 0;
	// dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	// dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	return init_render_pass(&attachment, 1, &subpass, 1, nullptr, 0);
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
	createInfo.attachmentCount = 1;
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

lvk_framebuffer::~lvk_framebuffer()
{
	// vkDestroyFramebuffer(device->_device, _framebuffer, VK_NULL_HANDLE);
	// dloggln("Framebuffer Destroyed");
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
	
	return semaphore;
}


lvk_semaphore::~lvk_semaphore()
{
	vkDestroySemaphore(device->_device, _semaphore, VK_NULL_HANDLE);
	dloggln("Semaphore Destroyed");
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

	return fence;
}

VkResult lvk_fence::wait(uint64_t timeout) {
	return vkWaitForFences(device->_device, 1, &_fence, false, timeout);
}

VkResult lvk_fence::reset() {
	return vkResetFences(device->_device, 1, &_fence);
}

lvk_fence::~lvk_fence()
{
	vkDestroyFence(device->_device, _fence, VK_NULL_HANDLE);
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
	
	VkShaderModuleCreateInfo info = lvk::shader_module_create_info(filename);

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


lvk_pipeline_layout lvk_device::init_pipeline_layout(const VkPushConstantRange* push_constant_range, uint32_t push_constant_range_count) {
	lvk_pipeline_layout pipeline_layout = {
		VK_NULL_HANDLE,
		this
	};

	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		0,
		VK_NULL_HANDLE,
		push_constant_range_count,
		push_constant_range
	};
	
	if (vkCreatePipelineLayout(this->_device, &pipeline_layout_create_info, VK_NULL_HANDLE, &pipeline_layout._pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("pipeline layout creation failed!");
	}
	dloggln("Pipeline Layout Created");
	
	return pipeline_layout;
}

lvk_pipeline_layout::~lvk_pipeline_layout()
{
	vkDestroyPipelineLayout(device->_device, _pipeline_layout, VK_NULL_HANDLE);
	dloggln("Pipeline Layout Destroyed");
}

lvk_pipeline lvk_pipeline_layout::init_graphics_pipeline(const lvk_render_pass* render_pass, const lvk::graphics_pipeline_config* config) {
	lvk_pipeline pipeline = {
		VK_NULL_HANDLE,
		this,
		render_pass,
		device
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
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.layout = this->_pipeline_layout;
	pipelineInfo.renderPass = render_pass->_render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	
	if (vkCreateGraphicsPipelines(device->_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline._pipeline) != VK_SUCCESS) {
		throw std::runtime_error("pipeline creation failed!");
	}
	dloggln("Pipeline Created");
	
	return pipeline;
}

lvk_pipeline::~lvk_pipeline()
{
	vkDestroyPipeline(device->_device, _pipeline, VK_NULL_HANDLE);
	dloggln("Pipeline Destroyed");
}


// |--------------------------------------------------
// ----------------> ALLOCATOR
// |--------------------------------------------------


lvk_allocator lvk_device::init_allocator() {
	lvk_allocator allocator = {
		VK_NULL_HANDLE,
		this,
		{}
	};
	
	VmaAllocatorCreateInfo allocatorInfo = {};

    allocatorInfo.physicalDevice = physical_device->_physical_device;
    allocatorInfo.device = _device;
    allocatorInfo.instance = instance->_instance;

    vmaCreateAllocator(&allocatorInfo, &allocator._allocator);
	dloggln("Allocator Created");
	
	return allocator;
}

lvk_allocator::~lvk_allocator() {
	deletion_queue.flush();

	vmaDestroyAllocator(_allocator);
	dloggln("Allocator Destroyed");
}


// |--------------------------------------------------
// ----------------> BUFFER
// |--------------------------------------------------


lvk_buffer lvk_allocator::init_buffer(VkBufferUsageFlagBits usage, const void* data, const std::size_t size, const VkSharingMode sharing_mode, const uint32_t* queue_family_indices, uint32_t queue_family_indices_count) {
	lvk_buffer buffer = {
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		this,
		size,
		usage
	};
	
	VkBufferCreateInfo bufferInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		size,
		usage,
		sharing_mode,
		queue_family_indices_count,
		queue_family_indices
	};

	//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	//allocate the buffer
	if (vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &buffer._buffer, &buffer._allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	dloggln("Buffer Created: ", lvk::to_string(usage));
	
	if (data != nullptr) {
		buffer.upload(data, size);
	}

	deletion_queue.push([=]{
		vmaDestroyBuffer(_allocator, buffer._buffer, buffer._allocation);
		dloggln("Buffer Destroyed: ", lvk::to_string(buffer.usage));
	});

	return buffer;
}

lvk_buffer lvk_allocator::init_vertex_buffer(const void* data, const std::size_t size) {
	return init_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data, size, VK_SHARING_MODE_EXCLUSIVE, nullptr, 0);
}

lvk_buffer lvk_allocator::init_vertex_buffer(const std::size_t size) {
	return init_vertex_buffer(nullptr, size);
}

void lvk_buffer::upload(const void* vertex_data, const std::size_t vertex_size) {
	if (vertex_size > allocated_size) {
		throw std::runtime_error("vertices size greater than allocated size!");
	}

	void* data;
	vmaMapMemory(allocator->_allocator, _allocation, &data);

	memcpy(data, vertex_data, vertex_size);

	vmaUnmapMemory(allocator->_allocator, _allocation);
}


// |--------------------------------------------------
// ----------------> IMAGE
// |--------------------------------------------------


lvk_image lvk_allocator::init_image(VkFormat format) {
	lvk_image image = {
		VK_NULL_HANDLE,
		VK_NULL_HANDLE,
		format,
		this,
		device
	};
	
	deletion_queue.push([=]{
		vmaDestroyImage(_allocator, image._image, image._allocation);
		dloggln("Image Destroyed");
	});
	
	return image;
}


// |--------------------------------------------------
// ----------------> IMAGE VIEW
// |--------------------------------------------------


lvk_image_view lvk_image::init_image_view(VkImageAspectFlags aspect_flag) {
	lvk_image_view image_view = {
		VK_NULL_HANDLE
	};

	VkImageViewCreateInfo createInfo = lvk::image_view_create_info(_image, _format, aspect_flag);

	if (vkCreateImageView(device->_device, &createInfo, VK_NULL_HANDLE, &image_view.image_view) != VK_SUCCESS) {
		throw std::runtime_error("image_view creation failed!");
	}
	dloggln("ImageView Created");

	deletion_queue->push([=]{
		vkDestroyImageView(device->_device, image_view.image_view, VK_NULL_HANDLE);
		dloggln("ImageView Destroyed");
	});

	return image_view;
}