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


lvk_device lvk_physical_device::init_device() {
	lvk_device device = {};
	
	device.instance = this->instance;
	device.physical_device = this;
	
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

void lvk_device::wait_idle() {
	vkDeviceWaitIdle(_device);
}

lvk_device::~lvk_device()
{
	vkDestroyDevice(_device, VK_NULL_HANDLE);
	dloggln("Device Destroyed");

	assert(swapchain_count == 0 && "swapchains are not destroyed");
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


lvk_swapchain* lvk_device::create_swapchain(uint32_t width, uint32_t height) {
	auto* self = new lvk_swapchain();

	self->device = this;
	self->physical_device = this->physical_device;
	self->instance = this->instance;

	self->_extent.width = width;
	self->_extent.height = height;

	const auto& present_modes = physical_device->_swapchain_support_details.present_modes;
	const auto& capabilities = physical_device->_swapchain_support_details.capabilities;

	self->_surface_format = get_swapchain_surface_format(self->physical_device->_swapchain_support_details.formats);

	uint32_t imageCount = (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) ? capabilities.maxImageCount: capabilities.minImageCount + 1;

	VkExtent2D extent = (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) ?
		capabilities.currentExtent:
		VkExtent2D {
			std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, self->_extent.width)),
			std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, self->_extent.height))
		};


	// TODO: support for more presentMode types
	// * VK_PRESENT_MODE_IMMEDIATE_KHR
	// * VK_PRESENT_MODE_MAILBOX_KHR = Mailbox
	// * VK_PRESENT_MODE_FIFO_KHR = V-sync
	// * VK_PRESENT_MODE_FIFO_RELAXED_KHR
	// * VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR
	// * VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR

	self->_present_mode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& availablePresentMode: present_modes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			self->_present_mode = availablePresentMode;
			break;
		}
		// if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
		// 	self->_present_mode = availablePresentMode;
		// 	break;
		// }
	}


	VkSwapchainCreateInfoKHR createInfo = {};
	{
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = instance->_surface;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = self->_surface_format.format;
		createInfo.imageColorSpace = self->_surface_format.colorSpace;
		createInfo.imageExtent = extent;
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

		createInfo.presentMode = self->_present_mode;

		// TODO: user defined clipping state
		createInfo.clipped = VK_TRUE;

		// TODO: remains to be tested
		createInfo.oldSwapchain = self->_swapchain;


		if (vkCreateSwapchainKHR(_device, &createInfo, VK_NULL_HANDLE, &self->_swapchain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swapchain!");
		}
		dloggln("Created Swapchain");
		
		swapchain_count += 1;
	}
	
	// ImageViews
	
	vkGetSwapchainImagesKHR(_device, self->_swapchain, &imageCount, nullptr);
	self->_images.resize(imageCount);
	self->_image_view_array.resize(self->_images.size());
	assert(vkGetSwapchainImagesKHR(_device, self->_swapchain, &imageCount, self->_images.data()) == VK_SUCCESS);

	for (size_t i = 0; i < self->_images.size(); i++) {
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		viewInfo.image = self->_images[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = self->_surface_format.format;

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(_device, &viewInfo, VK_NULL_HANDLE, &self->_image_view_array[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}
	}
	dloggln("ImageViews Created");
	
	return self;
}

uint32_t lvk_swapchain::acquire_next_image(VkSemaphore semaphore, VkFence fence, const uint64_t timeout) {
	uint32_t index;
	vkAcquireNextImageKHR(device->_device, _swapchain, timeout, semaphore, fence, &index);
	return index;
}

lvk_swapchain::~lvk_swapchain()
{
	vkDestroySwapchainKHR(device->_device, _swapchain, VK_NULL_HANDLE);
	dloggln("Swapchain Destroyed");

	for (int i = 0; i < _image_view_array.size(); i++) {
		vkDestroyImageView(device->_device, _image_view_array[i], VK_NULL_HANDLE);
	}
	dloggln("ImageViews Destroyed");
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


lvk_command_buffer lvk_command_pool::init_command_buffer(uint32_t count, VkCommandBufferLevel level) {
	lvk_command_buffer self = {
		new VkCommandBuffer[count],
		count,
		instance,
		physical_device,
		device,
		this
	};

	VkCommandBufferAllocateInfo allocateInfo = {};

	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.commandPool = _command_pool;
	allocateInfo.level = level;
	allocateInfo.commandBufferCount = count;
	if (vkAllocateCommandBuffers(device->_device, &allocateInfo, self._command_buffers) != VK_SUCCESS) {
		throw std::runtime_error("command buffer allocation failed!");
	}
	dloggln("Command Buffer Allocated: ", &self._command_buffers);
	
	return self;
}

void lvk_command_buffer::reset(const uint32_t index, VkCommandBufferResetFlags flags) {
	
	vkResetCommandBuffer(_command_buffers[index], flags);
}

void lvk_command_buffer::reset_all(VkCommandBufferResetFlags flags) {
	for (int i = 0; i < _count; i++)
		vkResetCommandBuffer(_command_buffers[i], flags);
}

void lvk_command_buffer::cmd_begin(const uint32_t index, const VkCommandBufferBeginInfo* beginInfo) {
	vkBeginCommandBuffer(_command_buffers[index], beginInfo);
}

void lvk_command_buffer::cmd_end(const uint32_t index) {
	vkEndCommandBuffer(_command_buffers[index]);
}

void lvk_command_buffer::cmd_render_pass_begin(const uint32_t index, const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents subpass_contents) {
	vkCmdBeginRenderPass(_command_buffers[index], beginInfo, subpass_contents);
}

void lvk_command_buffer::cmd_render_pass_end(const uint32_t index) {
	vkCmdEndRenderPass(_command_buffers[index]);
}

lvk_command_buffer::~lvk_command_buffer()
{
	vkFreeCommandBuffers(device->_device, command_pool->_command_pool, _count, _command_buffers);
	dloggln("Command Buffer Destroyed");
	delete [] _command_buffers;
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
		instance
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
	vkDestroyRenderPass(device->_device, _render_pass, VK_NULL_HANDLE);
	dloggln("RenderPass Destroyed");
}

// VkRenderPassBeginInfo lvk_render_pass::begin_info(VkFramebuffer framebuffer) {
// 	return {
// 		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
// 		VK_NULL_HANDLE,
// 		_render_pass,
// 		framebuffer,
// 		render_area,
// 		clear_values.size(),
// 		clear_values.data()
// 	}
// }


// |--------------------------------------------------
// ----------------> SEMAPHORE
// |--------------------------------------------------


lvk_semaphore lvk_device::init_semaphore(const uint32_t count, VkSemaphoreCreateFlags flags) {
	lvk_semaphore semaphore = {
		new VkSemaphore[count],
		count,
		this
	};

	VkSemaphoreCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = flags;

	for (int i = 0; i < count; i++) {
		if (vkCreateSemaphore(_device, &createInfo, VK_NULL_HANDLE, &semaphore._semaphore[i]) != VK_SUCCESS) {
			throw std::runtime_error("semaphore creation failed");
		}
	}
	dloggln("Semaphore Created");
	
	return semaphore;
}


lvk_semaphore::~lvk_semaphore()
{
	for (int i = 0; i < _count; i++) {
		vkDestroySemaphore(device->_device, _semaphore[i], VK_NULL_HANDLE);
	}
	dloggln("Semaphore Destroyed");
	
	delete [] _semaphore;
}


// |--------------------------------------------------
// ----------------> FENCE
// |--------------------------------------------------


lvk_fence lvk_device::init_fence(const uint32_t count, VkFenceCreateFlags flags) {
	lvk_fence fence = {
		new VkFence[count],
		count,
		this
	};

	VkFenceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = flags;

	for (int i = 0; i < count; i++) {
		if (vkCreateFence(_device, &createInfo, VK_NULL_HANDLE, &fence._fence[i]) != VK_SUCCESS) {
			throw std::runtime_error("fence creation failed");
		}
	}
	dloggln("Fence Created");

	return fence;
}

VkResult lvk_fence::wait(bool wait_all, uint64_t timeout) {
	return vkWaitForFences(device->_device, _count, _fence, wait_all, timeout);
}

VkResult lvk_fence::reset() {
	return vkResetFences(device->_device, _count, _fence);
}

lvk_fence::~lvk_fence()
{
	for (int i = 0; i < _count; i++)
		vkDestroyFence(device->_device, _fence[i], VK_NULL_HANDLE);
	dloggln("Fence Destroyed");
	
	delete [] _fence;
}


// |--------------------------------------------------
// ----------------> FRAMEBUFFER
// |--------------------------------------------------


lvk_framebuffer* lvk_render_pass::create_framebuffer(uint32_t width, uint32_t height, const std::vector<VkImageView>& image_view_array) {
	auto* self = new lvk_framebuffer();

	self->render_pass = this;
	self->device = this->device;

	self->_framebuffer_array.resize(image_view_array.size());
	
	VkFramebufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = _render_pass;
	createInfo.attachmentCount = 1;
	createInfo.width = width;
	createInfo.height = height;
	createInfo.layers = 1;

	for (int i = 0; i < self->_framebuffer_array.size(); i++) {
		createInfo.pAttachments = &image_view_array[i];
		if (vkCreateFramebuffer(device->_device, &createInfo, VK_NULL_HANDLE, &self->_framebuffer_array[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer");
		}
	}
	dloggln("Framebuffers Created");

	return self;
}

lvk_framebuffer::~lvk_framebuffer()
{
	for (auto& framebuffer: _framebuffer_array) {
		vkDestroyFramebuffer(device->_device, framebuffer, VK_NULL_HANDLE);
	}
	dloggln("Framebuffers Destroyed");
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


// lvk_graphics_shaders::~lvk_graphics_shaders()
// {
// 	if (_vertex_shader != VK_NULL_HANDLE) {
// 		vkDestroyShaderModule(device->_device, _vertex_shader, VK_NULL_HANDLE);
// 	}
// 	if (_testallation_control_shader != VK_NULL_HANDLE) {
// 		vkDestroyShaderModule(device->_device, _testallation_control_shader, VK_NULL_HANDLE);
// 	}
// 	if (_testallation_evaluation_shader != VK_NULL_HANDLE) {
// 		vkDestroyShaderModule(device->_device, _testallation_evaluation_shader, VK_NULL_HANDLE);
// 	}
// 	if (_geometry_shader != VK_NULL_HANDLE) {
// 		vkDestroyShaderModule(device->_device, _geometry_shader, VK_NULL_HANDLE);
// 	}
// 	if (_fragment_shader != VK_NULL_HANDLE) {
// 		vkDestroyShaderModule(device->_device, _fragment_shader, VK_NULL_HANDLE);
// 	}
// 	dloggln("Graphics Shaders Destroyed");
// }


lvk_pipeline_layout lvk_device::init_pipeline_layout() {
	lvk_pipeline_layout pipeline_layout = {
		VK_NULL_HANDLE,
		this
	};
	
	// VkDescriptorSetLayoutCreateInfo info = {
	// 	VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	// 	VK_NULL_HANDLE;
	// 	0;
	// 	uint32_t                               bindingCount;
	// 	const VkDescriptorSetLayoutBinding*    pBindings;
	// };
	
	VkPipelineLayoutCreateInfo pipeline_layout_create_info = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		VK_NULL_HANDLE,
		0,
		0,
		VK_NULL_HANDLE,
		0,
		VK_NULL_HANDLE
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


	// lvk_pipeline pipeline = {
	// 	VK_NULL_HANDLE,
	// 	this->device,
	// };
	
	// // VkPipelineLayoutCreateInfo layoutInfo = {
	// // 	VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	// // 	VK_NULL_HANDLE,
	// // 	0,
	// // 	layout_count,
	// // 	set_layout
	// // }

	// VkGraphicsPipelineCreateInfo graphicsCreateInfo = {};
	// graphicsCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// graphicsCreateInfo.pNext = VK_NULL_HANDLE;
	// graphicsCreateInfo.flags = 0;

	// graphicsCreateInfo.stageCount = config->shader_stage_array.size();
	// graphicsCreateInfo.pStages = config->shader_stage_array.data();

	// graphicsCreateInfo.pVertexInputState = &config->vertex_input_state;
	// graphicsCreateInfo.pInputAssemblyState = &config->input_assembly_state;
	// // graphicsCreateInfo.pTessellationState = &config->tessellation_state;
	// // graphicsCreateInfo.pViewportState = &config->viewport_state;
	// graphicsCreateInfo.pRasterizationState = &config->rasterization_state;
	// graphicsCreateInfo.pMultisampleState = &config->multisample_state;
	// // graphicsCreateInfo.pDepthStencilState = &config->depth_stencil_state;
	// graphicsCreateInfo.pColorBlendState = &config->color_blend_state;
	// // graphicsCreateInfo.pDynamicState = &config->dynamic_state;

	// graphicsCreateInfo.layout;
	// graphicsCreateInfo.renderPass = this->_render_pass;
	// graphicsCreateInfo.subpass = 0;
	// graphicsCreateInfo.basePipelineHandle;
	// graphicsCreateInfo.basePipelineIndex;
	
	// VkComputePipelineCreateInfo a;

	// return pipeline;

