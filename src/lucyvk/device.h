#pragma once

#include "lucyvk/pipeline.h"
#include "lucyvk/descriptor.h"
#include "lucyvk/render_pass.h"
#include "lucyvk/command.h"
#include "lucyvk/synchronization.h"
#include "lucyvk/texture.h"



// |--------------------------------------------------
// ----------------> DEVICE
// |--------------------------------------------------


struct lvk_device {
	VkDevice _device;
	
	VkQueue _present_queue;
	VkQueue _compute_queue;
	VkQueue _transfer_queue;
	VkQueue _graphics_queue;

	std::vector<const char*> extensions = {};
	std::vector<const char*> layers = {};

	lvk_device() {}
	lvk_device(lvk_instance* _instance, std::vector<const char*> extensions, std::vector<const char*> layers, lvk::SelectPhysicalDeviceFunction function);
	
	struct {
		VkPhysicalDevice _physical_device;

		VkPhysicalDeviceFeatures _features;
		VkPhysicalDeviceProperties _properties;

		lvk::queue_family_indices _queue_family_indices;
		lvk::swapchain_support_details _swapchain_support_details;

		const VkFormat find_supported_format(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		const uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags property_flags) const;
	} physical_device;
	
	void destroy();

	// const lvk_physical_device* physical_device;
	const lvk_instance* instance;
	
	lvk::deletion_queue deletion_queue;
	
	~lvk_device();


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

	lvk_command_buffer allocate_command_buffer_unique(const lvk_command_pool& command_pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	std::vector<lvk_command_buffer> allocate_command_buffers(const lvk_command_pool& command_pool, uint32_t command_buffer_count, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);


	// SWAPCHAIN 	---------- ---------- ---------- ----------


	lvk_swapchain create_swapchain(uint32_t width, uint32_t height, VkImageUsageFlags image_usage_flags, VkSurfaceFormatKHR surface_format);
	void swapchain_recreate(lvk_swapchain& swapchain, uint32_t width, uint32_t height) const;
	VkResult swapchain_acquire_next_image(const lvk_swapchain& swapchain, uint32_t* index, VkSemaphore semaphore, VkFence fence, const uint64_t timeout = LVK_TIMEOUT) const;
	
	
	// PIPELINE		---------- ---------- ---------- ----------
	
	
	lvk_pipeline_layout create_pipeline_layout(const VkPushConstantRange* push_constant_ranges = VK_NULL_HANDLE, const uint32_t push_constant_range_count = 0, const VkDescriptorSetLayout* descriptor_set_layouts = VK_NULL_HANDLE, const uint32_t descriptor_set_layout_count = 0);
	
	lvk_pipeline create_graphics_pipeline(const lvk_pipeline_layout& pipeline_layout, const lvk::config::graphics_pipeline* config, const lvk_render_pass* render_pass);
	lvk_pipeline create_compute_pipeline(const lvk_pipeline_layout& pipeline_layout, const VkPipelineShaderStageCreateInfo stage_info);

	// lvk_pipeline_layout init_pipeline_layout(const VkPushConstantRange* push_constant_ranges = VK_NULL_HANDLE, const uint32_t push_constant_range_count = 0, const VkDescriptorSetLayout* descriptor_set_layouts = VK_NULL_HANDLE, const uint32_t descriptor_set_layout_count = 0);

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
	

	// ALLOCATOR	---------- ---------- ---------- ----------
	
	lvk_allocator create_allocator();
	

	// IMAGE_VIEW	---------- ---------- ---------- ----------
	
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
	void update_descriptor_set(const lvk_descriptor_set& descriptor_set, uint32_t binding, const lvk_image_view* image_view, VkDescriptorType type, const std::size_t offset = 0) const;
	// template <std::size_t _ds_N> [[nodiscard, __gnu__::__always_inline__]]
	// constexpr lvk_descriptor_set create_descriptor_set(const lvk_descriptor_pool& descriptor_pool, const lvk_descriptor_set_layout& descriptor_set_layout)


	// DESCRIPTOR_POOL		   ---------- ---------- ----------

	lvk_descriptor_pool create_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize* descriptor_pool_sizes, const uint32_t descriptor_pool_sizes_count);

	template <std::size_t _dps_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_descriptor_pool create_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize (&descriptor_pool_sizes)[_dps_N]) noexcept {
		return create_descriptor_pool(max_descriptor_sets, descriptor_pool_sizes, _dps_N);
	}

	void clear_descriptor_pool(const lvk_descriptor_pool& descriptor_pool) const;
	void destroy_descriptor_pool(const lvk_descriptor_pool& descriptor_pool) const;


	// RENDERPASS	---------- ---------- ---------- ----------


	lvk_render_pass create_default_render_pass(VkFormat format);
	lvk_render_pass create_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform = false);
	// TODO: why ???
	template <std::size_t _ad_N, std::size_t _sdn_N, std::size_t _sdc_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_render_pass create_render_pass(const VkAttachmentDescription (&attachment)[_ad_N], const VkSubpassDescription (&subpass)[_sdn_N], const VkSubpassDependency (&dependency)[_sdc_N], bool enable_transform = false) noexcept {
		return create_render_pass(attachment, _ad_N, subpass, _sdn_N, dependency, _sdc_N);
	}
	
	
	// FRAMEBUFFER	---------- ---------- ---------- ----------
	
	
	lvk_framebuffer create_framebuffer(const lvk_render_pass& render_pass, const VkExtent2D extent, const VkImageView* image_views, const uint32_t image_views_count);
	
	template <std::size_t _iv_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_framebuffer create_framebuffer(const lvk_render_pass& render_pass, const VkExtent2D extent, const VkImageView (&image_views)[_iv_N]) noexcept {
		return create_framebuffer(render_pass, extent, image_views, _iv_N);
	}
	
	
	// 	 ---------- ---------- ---------- ---------- ----------

	// lvk_command_pool init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags);


	lvk_sampler init_sampler(VkFilter min_filter, VkFilter mag_filter, VkSamplerAddressMode sampler_addres_mode);
	
	lvk_queue init_queue();

	// lvk_semaphore init_semaphore();
	// lvk_fence init_fence(VkFenceCreateFlags flags = 0);

	lvk_shader_module init_shader_module(VkShaderStageFlagBits stage, const char* filename);
	
	
	// lvk_allocator init_allocator();
	
	// lvk_descriptor_set_layout init_descriptor_set_layout(const VkDescriptorSetLayoutBinding* bindings, const uint32_t binding_count);

	// template <std::size_t _ad_N, std::size_t _sdn_N, std::size_t _sdc_N>
	// lvk_render_pass init_render_pass(const VkAttachmentDescription (&attachment)[_ad_N], const VkSubpassDescription (&subpass)[_sdn_N], const VkSubpassDependency (&dependency)[_sdc_N], bool enable_transform = false);

	void wait_idle() const;


	VkResult submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const VkFence fence, uint64_t timeout = LVK_TIMEOUT) const;
	// VkResult submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const VkFence fence, uint64_t timeout = LVK_TIMEOUT) const;

	VkResult submit2(const VkSubmitInfo2* submit_info2, const uint32_t submit_info2_count, const VkFence fence) const;
	
	template <std::size_t _si2_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkResult submit2(const VkSubmitInfo2 (&submit_info2)[_si2_N], const lvk_fence* fence) noexcept {
		return submit2(submit_info2, _si2_N, fence);
	}

	VkResult present(const VkPresentInfoKHR* present_info) const;



	// VkSemaphore* create_semaphore();
};