#include "descriptors.h"
#include "device.h"

lvk_descriptor_pool lvk_descriptor_allocator_growable::get_pool() {
    lvk_descriptor_pool new_pool;
    if (ready_pools.size() != 0) {
        new_pool = ready_pools.back();
        ready_pools.pop_back();
    }
    else {
	    //need to create a new pool
	    new_pool = create_pool(sets_per_pool, ratios);

	    sets_per_pool = sets_per_pool * 1.5;
	    if (sets_per_pool > 4092) {
		    sets_per_pool = 4092;
	    }
    }

    return new_pool;
}

lvk_descriptor_pool lvk_descriptor_allocator_growable::create_pool(uint32_t set_count, std::span<lvk_pool_size_ratio> pool_ratios) {
	std::vector<VkDescriptorPoolSize> pool_sizes = {};

	for (lvk_pool_size_ratio ratio: pool_ratios) {
		pool_sizes.push_back({
			.type = ratio.type,
			.descriptorCount = uint32_t(ratio.ratio * set_count)
		});
	}

	return device->create_descriptor_pool(set_count, pool_sizes.data(), pool_sizes.size());
}

void lvk_descriptor_allocator_growable::init(uint32_t max_sets, std::span<lvk_pool_size_ratio> pool_ratios) {
    ratios.clear();

    for (auto r: pool_ratios) {
        ratios.push_back(r);
    }

    lvk_descriptor_pool new_pool = create_pool(max_sets, pool_ratios);

    sets_per_pool = max_sets * 1.5; //grow it next allocation

    ready_pools.push_back(new_pool);
}

void lvk_descriptor_allocator_growable::clear_pools() {
    for (auto p: ready_pools) {
        vkResetDescriptorPool(device->get_logical_device(), p, 0);
    }
    for (auto p: full_pools) {
        vkResetDescriptorPool(device->get_logical_device(), p, 0);
        ready_pools.push_back(p);
    }
    full_pools.clear();
}

void lvk_descriptor_allocator_growable::destroy_pools() {
	for (auto p: ready_pools) {
		vkDestroyDescriptorPool(device->get_logical_device(), p, nullptr);
	}
    ready_pools.clear();
	for (auto p: full_pools) {
		vkDestroyDescriptorPool(device->get_logical_device(), p, nullptr);
    }
    full_pools.clear();
}

lvk_descriptor_set lvk_descriptor_allocator_growable::allocate(VkDescriptorSetLayout layout, void* pNext) {
    //get or create a pool to allocate from
    lvk_descriptor_pool pool_to_use = get_pool();

	VkDescriptorSetAllocateInfo alloc_info = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = pNext,

		.descriptorPool = pool_to_use,
		.descriptorSetCount = 1,
		.pSetLayouts = &layout,
	};

	VkDescriptorSet descriptor_set = VK_NULL_HANDLE;
	VkResult result = vkAllocateDescriptorSets(device->get_logical_device(), &alloc_info, &descriptor_set);

    //allocation failed. Try again
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        full_pools.push_back(pool_to_use);

        pool_to_use = get_pool();
        alloc_info.descriptorPool = pool_to_use;

       vkAllocateDescriptorSets(device->get_logical_device(), &alloc_info, &descriptor_set);
    }

    ready_pools.push_back(pool_to_use);
    return {
		._descriptor_set = descriptor_set
	};
}

void lvk_descriptor_writer::write_image(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type) {
	VkDescriptorImageInfo& info = image_infos.emplace_back(
		VkDescriptorImageInfo {
			.sampler = sampler,
			.imageView = image,
			.imageLayout = layout
		}
	);

	VkWriteDescriptorSet write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,

		.dstSet = VK_NULL_HANDLE, //left empty for now until we need to write it
		.dstBinding = static_cast<uint32_t>(binding),

		.descriptorCount = 1,
		.descriptorType = type,

		.pImageInfo = &info,
	};

	writes.push_back(write);
}

void lvk_descriptor_writer::write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type) {
	VkDescriptorBufferInfo& info = buffer_infos.emplace_back(
		VkDescriptorBufferInfo {
			.buffer = buffer,
			.offset = offset,
			.range = size
		}
	);

	VkWriteDescriptorSet write = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,

		.dstSet = VK_NULL_HANDLE, //left empty for now until we need to write it
		.dstBinding = static_cast<uint32_t>(binding),

		.descriptorCount = 1,
		.descriptorType = type,

		.pBufferInfo = &info,
	};

	writes.push_back(write);
}

void lvk_descriptor_writer::clear() {
    image_infos.clear();
    writes.clear();
    buffer_infos.clear();
}

void lvk_descriptor_writer::update_set(VkDescriptorSet set) {
	for (VkWriteDescriptorSet& write: writes) {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(device->get_logical_device(), (uint32_t)writes.size(), writes.data(), 0, nullptr);
}
