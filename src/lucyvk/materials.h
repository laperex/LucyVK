#pragma once

#include "lucyvk/descriptors.h"
#include <lucylm/math.hpp>


// Render Architecture
enum class MaterialPass: uint8_t {
    MainColor,
    Transparent,
    Other
};

struct MaterialPipeline {
	lvk_pipeline pipeline;
	lvk_pipeline_layout layout;
};

struct MaterialInstance {
    MaterialPipeline* pipeline;
    lvk_descriptor_set material_set;
    MaterialPass pass_type;
};

// base class for a renderable dynamic object
class IRenderable {
    virtual void Draw(const glm::mat4& top_matrix, DrawContext& ctx) = 0;
};


struct RenderObject {
    uint32_t indexCount;
    uint32_t firstIndex;
    VkBuffer indexBuffer;
    
    MaterialInstance* material;

    glm::mat4 transform;
    VkDeviceAddress vertexBufferAddress;
};

struct GLTFMetallic_Roughness {
	MaterialPipeline opaquePipeline;
	MaterialPipeline transparentPipeline;

	VkDescriptorSetLayout materialLayout;

	struct MaterialConstants {
		glm::vec4 colorFactors;
		glm::vec4 metal_rough_factors;
		//padding, we need it anyway for uniform buffers
		glm::vec4 extra[14];
	};

	struct MaterialResources {
		lvk_image color_image;
		lvk_sampler color_sampler;

		lvk_image metal_rough_image;
		lvk_sampler metal_rough_sampler;

		VkBuffer data_buffer;
		uint32_t data_buffer_offset;
	};

	lvk_descriptor_writer writer;

	void build_pipelines();
	void clear_resources();

	MaterialInstance write_material(MaterialPass pass, const MaterialResources& resources, lvk_descriptor_allocator_growable& descriptor_allocator);
};