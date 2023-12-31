#pragma once

#include "vk_static.h"


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
	
	~lvk_device();

	const lvk_physical_device* physical_device;
	const lvk_instance* instance;
	
	lvk::deletion_queue deletion_queue;

	lvk_swapchain init_swapchain(uint32_t width, uint32_t height, VkImageUsageFlags image_usage_flags, VkSurfaceFormatKHR surface_format);
	
	lvk_command_pool init_command_pool();
	lvk_command_pool init_command_pool(uint32_t queue_family_index, VkCommandPoolCreateFlags flags);
	
	lvk_queue init_queue();
	
	lvk_render_pass init_render_pass(const VkAttachmentDescription* attachment, uint32_t attachment_count, const VkSubpassDescription* subpass, const uint32_t subpass_count, const VkSubpassDependency* dependency, const uint32_t dependency_count, bool enable_transform = false);
	lvk_render_pass init_default_render_pass(VkFormat format);

	lvk_semaphore init_semaphore();
	lvk_fence init_fence(VkFenceCreateFlags flags = 0);

	lvk_shader_module init_shader_module(VkShaderStageFlagBits stage, const char* filename);
	
	lvk_pipeline_layout init_pipeline_layout(const VkPushConstantRange* push_constant_ranges, const uint32_t push_constant_range_count, const VkDescriptorSetLayout* descriptor_set_layouts, const uint32_t descriptor_set_layout_count);
	
	lvk_allocator init_allocator();
	
	lvk_descriptor_set_layout init_descriptor_set_layout(const VkDescriptorSetLayoutBinding* bindings, const uint32_t binding_count);
	lvk_descriptor_pool init_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize* descriptor_pool_sizes, const uint32_t descriptor_pool_sizes_count);

	// TODO: why ???
	template <std::size_t _ad_N, std::size_t _sdn_N, std::size_t _sdc_N>
	lvk_render_pass init_render_pass(const VkAttachmentDescription (&attachment)[_ad_N], const VkSubpassDescription (&subpass)[_sdn_N], const VkSubpassDependency (&dependency)[_sdc_N], bool enable_transform = false);

	template <std::size_t _pcr_N, std::size_t _dsl_N> [[nodiscard, __gnu__::__always_inline__]]
    constexpr lvk_pipeline_layout init_pipeline_layout(const VkPushConstantRange (&push_constant_ranges)[_pcr_N], const VkDescriptorSetLayout (&descriptor_set_layouts)[_dsl_N]) noexcept {
		return init_pipeline_layout(push_constant_ranges, _pcr_N, descriptor_set_layouts, _dsl_N);
	}
	
	template <std::size_t _pcr_N> [[nodiscard, __gnu__::__always_inline__]]
    constexpr lvk_pipeline_layout init_pipeline_layout(const VkPushConstantRange (&push_constant_ranges)[_pcr_N]) noexcept {
		return init_pipeline_layout(push_constant_ranges, _pcr_N, VK_NULL_HANDLE, 0);
	}
	
	template <std::size_t _dsl_N> [[nodiscard, __gnu__::__always_inline__]]
    constexpr lvk_pipeline_layout init_pipeline_layout(const VkDescriptorSetLayout (&descriptor_set_layouts)[_dsl_N]) noexcept {
		return init_pipeline_layout(VK_NULL_HANDLE, 0, descriptor_set_layouts, _dsl_N);
	}
	
	template <std::size_t _dslb_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_descriptor_set_layout init_descriptor_set_layout(const VkDescriptorSetLayoutBinding (&bindings)[_dslb_N]) noexcept {
		return init_descriptor_set_layout(bindings, _dslb_N);
	}

	template <std::size_t _dps_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr lvk_descriptor_pool init_descriptor_pool(const uint32_t max_descriptor_sets, const VkDescriptorPoolSize (&descriptor_pool_sizes)[_dps_N]) noexcept {
		return init_descriptor_pool(max_descriptor_sets, descriptor_pool_sizes, _dps_N);
	}

	void wait_idle() const;

	VkResult submit(const VkSubmitInfo* submit_info, uint32_t submit_count, const lvk_fence* fence, uint64_t timeout = LVK_TIMEOUT) const;

	VkResult submit2(const VkSubmitInfo2* submit_info2, const uint32_t submit_info2_count, const lvk_fence* fence) const;
	
	template <std::size_t _si2_N> [[nodiscard, __gnu__::__always_inline__]]
	constexpr VkResult submit2(const VkSubmitInfo2 (&submit_info2)[_si2_N], const lvk_fence* fence) noexcept {
		return submit2(submit_info2, _si2_N, fence);
	}

	VkResult present(const VkPresentInfoKHR* present_info) const;
};
