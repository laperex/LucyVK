#pragma once

#include <map>
#include <optional>
#include <deque>

#include "lucyvk/define.h"
#include "lucyvk/destroyer.h"
#include "lucyvk/handles.h"
// #include "lucyvk/types.h"
#include "lucyvk/command.h"
#include "lucyvk/config.h"



// |--------------------------------------------------
// ----------------> DEVICE
// |--------------------------------------------------


struct lvk_device {
	LVK_HANDLE_DEF(VkDevice, _device)
	
	std::vector<const char*> extensions;

	struct {
		struct {
			LVK_HANDLE_DEF(VkQueue, handle)
			std::optional<uint32_t> index;
		} graphics, present, compute, transfer;

		operator bool() const {
			return graphics.index.has_value() && present.index.has_value() && compute.index.has_value() && transfer.index.has_value();
		}
	} _queue;

	struct {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> present_modes;
	} _swapchain_support_details;

	struct {
		LVK_HANDLE_DEF(VkPhysicalDevice, _physical_device)

		VkPhysicalDeviceFeatures _features;
		VkPhysicalDeviceProperties _properties;

		const VkFormat find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		const uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags property_flags) const;
	} physical_device;

	struct {
		LVK_HANDLE_DEF(VmaAllocator, _allocator)
	} allocator;

	lvk_destroyer destroyer;

	VkSurfaceKHR _surfaceKHR;

	void destroy_device();


	// ~lvk_device();
	
	// DESTROYER 		---------- ---------- ---------- ----------
	
	void destroy(VkCommandPool command_pool);
	void destroy(VkPipelineLayout pipeline_layout);
	void destroy(VkPipeline pipeline);
	void destroy(VkSwapchainKHR swapchain);
	void destroy(VkSemaphore semaphore);
	void destroy(VkFence fence);
	void destroy(VkDescriptorSetLayout descriptor_set_layout);
	void destroy(VkDescriptorPool descriptor_pool);
	void destroy(VkFramebuffer framebuffer);
	void destroy(VkRenderPass render_pass);
	void destroy(VkImageView image_view);
	void destroy(VkShaderModule shader_module);
	void destroy(VkSampler sampler);
	
	// void destroy(VkCommandBuffer command_buffer, VkCommandPool command_pool);
	void destroy(VkCommandBuffer* command_buffer, uint32_t command_buffer_count, VkCommandPool command_pool);
	// void destroy(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool);
	// void destroy(VkDescriptorSet* descriptor_set, uint32_t descriptor_set_count, VkDescriptorPool descriptor_pool);

	void destroy(VkBuffer buffer, VmaAllocation allocation);
	void destroy(VkImage image, VmaAllocation allocation);


	// SYNCHRONIZATION 		---------- ---------- ---------- ----------


	lvk_semaphore create_semaphore();

	lvk_fence create_fence(VkFenceCreateFlags flags = 0);


	void wait_for_fence(const lvk_fence& fence, uint64_t timeout = LVK_TIMEOUT) const;
	void wait_for_fences(const lvk_fence* fence, uint32_t fence_count, uint64_t timeout = LVK_TIMEOUT) const;

	template <std::size_t _f_N>
	void wait_for_fences(const lvk_fence (&fence)[_f_N], uint64_t timeout = LVK_TIMEOUT) {
		wait_for_fences(fence, _f_N, timeout);
	}


	void reset_fence(const lvk_fence& fence) const;
	void reset_fences(const lvk_fence* fence, uint32_t fence_count) const;

	template <std::size_t _f_N>
	void reset_fences(const lvk_fence (&fence)[_f_N]) {
		reset_fences(fence, _f_N);
	}


	// COMMAND 		---------- ---------- ---------- ----------


	lvk_command_pool create_graphics_command_pool();
	lvk_command_pool create_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags);
	void reset_command_pool(const lvk_command_pool& command_pool);

	lvk_command_buffer create_command_buffer_unique(const VkCommandPool command_pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	std::vector<lvk_command_buffer> create_command_buffers(const lvk_command_pool& command_pool, uint32_t command_buffer_count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	
	// lvk_immediate_command create_immediate_command();
	VkResult imm_submit(std::function<void(const VkCommandBuffer)> function);
	// VkResult imm_buffer_copy(const VkBuffer src_buffer, const VkBuffer dst_buffer, const VkDeviceSize size);
	// VkResult 


	// SWAPCHAIN 	---------- ---------- ---------- ----------


	lvk_swapchain create_swapchain(VkRenderPass render_pass, uint32_t width, uint32_t height, VkImageUsageFlags image_usage_flags, VkSurfaceFormatKHR surface_format);
	void swapchain_recreate(lvk_swapchain& swapchain, const VkRenderPass render_pass, uint32_t width, uint32_t height);
	VkResult swapchain_acquire_next_image(const lvk_swapchain& swapchain, uint32_t* index, VkSemaphore semaphore, VkFence fence, const uint64_t timeout = LVK_TIMEOUT) const;
	
	
	// SHADER		---------- ---------- ---------- ----------
	

	lvk_shader_module create_shader_module(const char* filename);
	
	
	// PIPELINE		---------- ---------- ---------- ----------
	
	
	lvk_pipeline_layout create_pipeline_layout(const VkPushConstantRange* push_constant_ranges = VK_NULL_HANDLE, const uint32_t push_constant_range_count = 0, const VkDescriptorSetLayout* descriptor_set_layouts = VK_NULL_HANDLE, const uint32_t descriptor_set_layout_count = 0);
	


	template <std::size_t _pcr_N, std::size_t _dsl_N> [[nodiscard, __gnu__::__always_inline__]]
    constexpr lvk_pipeline_layout create_pipeline_layout(const VkPushConstantRange (&push_constant_ranges)[_pcr_N], const VkDescriptorSetLayout (&descriptor_set_layouts)[_dsl_N]) noexcept {
		return create_pipeline_layout(push_constant_ranges, _pcr_N, descriptor_set_layouts, _dsl_N);
	}
	
	template <std::size_t _pcr_N> [[nodiscard, __gnu__::__always_inline__]]
    constexpr lvk_pipeline_layout create_pipeline_layout(const VkPushConstantRange (&push_constant_ranges)[_pcr_N]) noexcept {
		return create_pipeline_layout(push_constant_ranges, _pcr_N, VK_NULL_HANDLE, 0);
	}
	
	template <std::size_t _dsl_N> [[nodiscard, __gnu__::__always_inline__]]
    constexpr lvk_pipeline_layout create_pipeline_layout(const VkDescriptorSetLayout (&descriptor_set_layouts)[_dsl_N]) noexcept {
		return create_pipeline_layout(VK_NULL_HANDLE, 0, descriptor_set_layouts, _dsl_N);
	}


	// template <std::size_t _pssci_N> [[nodiscard, __gnu__::__always_inline__]]
	// constexpr lvk_pipeline create_graphics_pipeline(
	// 	const VkPipelineLayout pipeline_layout,
	// 	const VkPipelineShaderStageCreateInfo (&shader_stages)[_pssci_N],
	// 	const VkPipelineVertexInputStateCreateInfo vertex_input_state,
	// 	const VkPipelineInputAssemblyStateCreateInfo input_assembly_state,
	// 	const VkPipelineTessellationStateCreateInfo testallation_state,
	// 	const VkPipelineViewportStateCreateInfo viewport_state,
	// 	const VkPipelineRasterizationStateCreateInfo rasterization_state,
	// 	const VkPipelineMultisampleStateCreateInfo multisample_state,
	// 	const VkPipelineDepthStencilStateCreateInfo depth_stencil_state,
	// 	const VkPipelineColorBlendStateCreateInfo color_blend_state,
	// 	const VkPipelineDynamicStateCreateInfo dynamic_state,
	// 	const VkRenderPass render_pass
	// ) noexcept {
	// 	return create_graphics_pipeline(
	// 		pipeline_layout,
	// 		shader_stages,
	// 		_pssci_N,
	// 		vertex_input_state,
	// 		input_assembly_state,
	// 		testallation_state,
	// 		viewport_state,
	// 		rasterization_state,
	// 		multisample_state,
	// 		depth_stencil_state,
	// 		color_blend_state,
	// 		dynamic_state,
	// 		render_pass
	// 	);
	// }
	
	// lvk_pipeline create_graphics_pipeline(
	// 	const VkPipelineLayout pipeline_layout,
	// 	const VkPipelineShaderStageCreateInfo* shader_stage_array,
	// 	const uint32_t shader_stage_array_size,
	// 	const VkPipelineVertexInputStateCreateInfo vertex_input_state,
	// 	const VkPipelineInputAssemblyStateCreateInfo input_assembly_state,
	// 	const VkPipelineTessellationStateCreateInfo testallation_state,
	// 	const VkPipelineViewportStateCreateInfo viewport_state,
	// 	const VkPipelineRasterizationStateCreateInfo rasterization_state,
	// 	const VkPipelineMultisampleStateCreateInfo multisample_state,
	// 	const VkPipelineDepthStencilStateCreateInfo depth_stencil_state,
	// 	const VkPipelineColorBlendStateCreateInfo color_blend_state,
	// 	const VkPipelineDynamicStateCreateInfo dynamic_state,
	// 	const VkRenderPass render_pass
	// );
	
	lvk_pipeline create_graphics_pipeline(const VkPipelineLayout pipeline_layout, const lvk::config::graphics_pipeline& config, const VkRenderPass render_pass);
	lvk_pipeline create_compute_pipeline(const VkPipelineLayout pipeline_layout, const VkPipelineShaderStageCreateInfo stage_info);
	

	// BUFFER	---------- ---------- ---------- ----------
	
	
	void upload(const VmaAllocation allocation, const VkDeviceSize size, const void* data) const;

	
	lvk_buffer create_buffer(const VkBufferUsageFlags buffer_usage, VmaMemoryUsage memory_usage, const VkDeviceSize size, const void* data = nullptr);
	
	
	lvk_buffer create_staging_buffer(const VkDeviceSize size, const void* data = nullptr);
	
	
	lvk_buffer create_index_buffer(const VkDeviceSize size, const void* data = nullptr);
	lvk_buffer create_index_buffer_static(const VkDeviceSize size, const void* data = nullptr);

	template <typename T>
	lvk_buffer create_index_buffer(const std::vector<T>& data) {
		return create_index_buffer(data.size() * sizeof(T), data.data());
	}
	template <typename T>
	lvk_buffer create_index_buffer_static(const std::vector<T>& data) {
		return create_index_buffer_static(data.size() * sizeof(T), data.data());
	}
	
	
	lvk_buffer create_vertex_buffer(const VkDeviceSize size, const void* data = nullptr);
	lvk_buffer create_vertex_buffer_static(const VkDeviceSize size, const void* data);

	template <typename T>
	lvk_buffer create_vertex_buffer(const std::vector<T>& data) {
		return create_vertex_buffer(data.size() * sizeof(T), data.data());
	}

	template <typename T, std::size_t N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_buffer create_vertex_buffer(const T (&data)[N]) noexcept {
		return create_vertex_buffer(sizeof(T) * N, data);
	}


	lvk_buffer create_uniform_buffer(const VkDeviceSize size, const void* data = nullptr);
	lvk_buffer create_uniform_buffer_static(const VkDeviceSize size, const void* data);

	template <typename T> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_buffer create_uniform_buffer(const T* data = nullptr) noexcept {
		return create_uniform_buffer(sizeof(T), data);
	}

	void upload(const lvk_buffer& buffer, const VkDeviceSize size, const void* data = nullptr, lvk_buffer staging_buffer = {});
	template <typename T>
	inline void upload(const lvk_buffer& buffer, const T& data) {
		upload(buffer, sizeof(T), &data);
	}

	// IMAGE

	lvk_image create_image(VkFormat format, VkImageUsageFlags usage, VmaMemoryUsage memory_usage, VkExtent3D extent, VkImageType image_type);
	

	// IMAGE_VIEW	---------- ---------- ---------- ----------
	
	lvk_image_view create_image_view(const VkImage image, VkFormat format, VkImageAspectFlags aspect_flag, VkImageViewType image_view_type);
	lvk_image_view create_image_view(const lvk_image& image, VkImageAspectFlags aspect_flag, VkImageViewType image_view_type);


	// DESCRIPTOR_SET_LAYOUT   ---------- ---------- ----------


	lvk_descriptor_set_layout create_descriptor_set_layout(const VkDescriptorSetLayoutBinding* bindings, const uint32_t binding_count);

	template <std::size_t _dsl_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_descriptor_set_layout create_descriptor_set_layout(const VkDescriptorSetLayoutBinding (&bindings)[_dsl_N]) noexcept {
		return create_descriptor_set_layout(bindings, _dsl_N);
	}
	
	
	// DESCRIPTOR_SET		   ---------- ---------- ----------
	
	lvk_descriptor_set create_descriptor_set(const lvk_descriptor_pool& descriptor_pool, const lvk_descriptor_set_layout& descriptor_set_layout);
	void update_descriptor_set(const lvk_descriptor_set& descriptor_set, uint32_t binding, const lvk_buffer* buffer, VkDescriptorType type, const std::size_t offset = 0) const;
	void update_descriptor_set(const lvk_descriptor_set& descriptor_set, uint32_t binding, const lvk_image_view* image_view, const lvk_sampler& sampler, VkImageLayout image_layout, VkDescriptorType type, const std::size_t offset = 0) const;
	// template <std::size_t _ds_N> [[nodiscard, __gnu__::__always_inline__]]
	// constexpr lvk_descriptor_set create_descriptor_set(const lvk_descriptor_pool& descriptor_pool, const lvk_descriptor_set_layout& descriptor_set_layout)


	// DESCRIPTOR_POOL		   ---------- ---------- ----------

	lvk_descriptor_pool create_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize* descriptor_pool_sizes, const uint32_t descriptor_pool_sizes_count);

	template <std::size_t _dps_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_descriptor_pool create_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize (&descriptor_pool_sizes)[_dps_N]) noexcept {
		return create_descriptor_pool(max_descriptor_sets, descriptor_pool_sizes, _dps_N);
	}

	void clear_descriptor_pool(const lvk_descriptor_pool& descriptor_pool) const;
	// void destroy_descriptor_pool(const lvk_descriptor_pool& descriptor_pool) const;


	// RENDERPASS	---------- ---------- ---------- ----------


	lvk_render_pass create_default_render_pass(VkFormat format);
	lvk_render_pass create_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform = false);
	// TODO: why ???
	template <std::size_t _ad_N, std::size_t _sdn_N, std::size_t _sdc_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_render_pass create_render_pass(const VkAttachmentDescription (&attachment)[_ad_N], const VkSubpassDescription (&subpass)[_sdn_N], const VkSubpassDependency (&dependency)[_sdc_N], bool enable_transform = false) noexcept {
		return create_render_pass(attachment, _ad_N, subpass, _sdn_N, dependency, _sdc_N);
	}
	
	
	// FRAMEBUFFER	---------- ---------- ---------- ----------
	
	
	lvk_framebuffer create_framebuffer(const VkRenderPass render_pass, const VkExtent2D extent, const VkImageView* image_views, const uint32_t image_views_count);
	
	template <std::size_t _iv_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_framebuffer create_framebuffer(const VkRenderPass render_pass, const VkExtent2D extent, const VkImageView (&image_views)[_iv_N]) noexcept {
		return create_framebuffer(render_pass, extent, image_views, _iv_N);
	}


	// SAMPLER	---------- ---------- ---------- ----------


	lvk_sampler create_sampler(VkFilter min_filter, VkFilter mag_filter, VkSamplerAddressMode sampler_addres_mode);
	
	// 	 ---------- ---------- ---------- ---------- ----------

	// lvk_command_pool init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags);


	
	// lvk_queue init_queue();

	// lvk_semaphore init_semaphore();
	// lvk_fence init_fence(VkFenceCreateFlags flags = 0);
	
	
	// lvk_allocator init_allocator();
	
	// lvk_descriptor_set_layout init_descriptor_set_layout(const VkDescriptorSetLayoutBinding* bindings, const uint32_t binding_count);

	// template <std::size_t _ad_N, std::size_t _sdn_N, std::size_t _sdc_N>
	// lvk_render_pass init_render_pass(const VkAttachmentDescription (&attachment)[_ad_N], const VkSubpassDescription (&subpass)[_sdn_N], const VkSubpassDependency (&dependency)[_sdc_N], bool enable_transform = false);

	void wait_idle() const;


	VkResult submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const VkFence fence, uint64_t timeout = LVK_TIMEOUT) const;

	template <std::size_t _si_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkResult submit(const VkSubmitInfo (&submit_info)[_si_N], const lvk_fence& fence, uint64_t timeout = LVK_TIMEOUT) const noexcept {
		return submit(submit_info, _si_N, fence, timeout);
	}
	// VkResult submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const VkFence fence, uint64_t timeout = LVK_TIMEOUT) const;

	VkResult submit2(const VkSubmitInfo2* submit_info2, const uint32_t submit_info2_count, const VkFence fence) const;
	template <std::size_t _si2_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkResult submit2(const VkSubmitInfo2 (&submit_info2)[_si2_N], const lvk_fence* fence) noexcept {
		return submit2(submit_info2, _si2_N, fence);
	}

	VkResult present(const VkPresentInfoKHR present_info) const;
	VkResult present(const uint32_t image_index, const VkSwapchainKHR* swapchains, const uint32_t swapchain_count, const VkSemaphore* semaphores, const uint32_t semaphore_count) const;
	VkResult present(const uint32_t image_index, const VkSwapchainKHR swapchain, const VkSemaphore semaphore) const;
	template <std::size_t _skhr_N, std::size_t _s_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkResult present(const uint32_t image_index, const VkSwapchainKHR (&swapchains)[_skhr_N], const VkSemaphore (&semaphores)[_s_N]) const noexcept {
		return present(image_index, swapchains, _skhr_N, semaphores, _s_N);
	}
};