#pragma once

#include <deque>
#include <span>
#include <vulkan/vulkan_core.h>
#include "handles.h"

struct lvk_descriptor_allocator_growable {
public:
	struct PoolSizeRatio {
		VkDescriptorType type;
		float ratio;
	};


private:
	const lvk_device* device;

	std::vector<PoolSizeRatio> ratios;
	std::vector<lvk_descriptor_pool> full_pools;
	std::vector<lvk_descriptor_pool> ready_pools;
	uint32_t sets_per_pool;

	lvk_descriptor_pool get_pool();
	lvk_descriptor_pool create_pool(uint32_t set_count, std::span<PoolSizeRatio> pool_ratios);

public:
	lvk_descriptor_allocator_growable(const lvk_device& device): device(&device) {}
	lvk_descriptor_allocator_growable(): device(VK_NULL_HANDLE) {}

	void init(uint32_t initial_sets, std::span<PoolSizeRatio> pool_ratios);
	void clear_pools();
	void destroy_pools();

    lvk_descriptor_set allocate(VkDescriptorSetLayout layout, void* pNext = nullptr);

	// lvk_descriptor_allocator_growable(const lvk_descriptor_allocator_growable &) = default;
	// lvk_descriptor_allocator_growable(lvk_descriptor_allocator_growable &&) = default;
	// lvk_descriptor_allocator_growable& operator=(const lvk_descriptor_allocator_growable &) = delete;
	// lvk_descriptor_allocator_growable& operator=(lvk_descriptor_allocator_growable &&) = delete;
	// lvk_descriptor_allocator_growable(std::vector<PoolSizeRatio> ratios, std::vector<VkDescriptorPool> fullPools, std::vector<VkDescriptorPool> readyPools, uint32_t setsPerPool, const lvk_device &device):
	// 	ratios(std::move(ratios)), fullPools(std::move(fullPools)), readyPools(std::move(readyPools)), setsPerPool(setsPerPool), device(device) {}
};


struct lvk_descriptor_writer {
private:
	std::deque<VkDescriptorImageInfo> image_infos;
	std::deque<VkDescriptorBufferInfo> buffer_infos;

	std::vector<VkWriteDescriptorSet> writes;

	const lvk_device* device;

public:
	lvk_descriptor_writer(const lvk_device& device): device(&device) {}
	lvk_descriptor_writer(): device(VK_NULL_HANDLE) {}

	void write_image(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
	void write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

	void clear();
	void update_set(VkDescriptorSet set);
};