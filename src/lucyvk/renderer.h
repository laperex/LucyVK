#pragma once

// #include "lucylm/math.hpp"

// #include "lucyvk/mesh.h"
#include "lucyvk/command.h"
#include "lucyvk/device.h"
#include "lucyvk/descriptors.h"
#include "lucyvk/handles.h"
#include "lucyvk/instance.h"
#include "lucyvk/functions.h"
// #include "lucyvk/memory.h"
// #include "lucyvk/synchronization.h"
// #include "lucyvk/types.h"
#include <filesystem>
#include <memory>
#include <vector>

#include <glm/gtc/quaternion.hpp>
// #include "lucyvk/render_pass.h"
// #include "lucyvk/swapchain.h"
// #include "lucyvk/pipeline_manager.h"

#define FRAMES_IN_FLIGHT 2


struct lre_frame {
	lvk_fence render_fence;

	lvk_semaphore present_semaphore;
	lvk_semaphore render_semaphore;

	lvk_command_buffer command_buffer;

	uint32_t swapchain_image_index;

	lvk_image draw_image = {};
	lvk_image_view draw_image_view = {};
	
	lvk_image depth_image = {};
	lvk_image_view depth_image_view = {};
	
	lvk_descriptor_allocator_growable descriptor_allocator;

	lvk_deletor_deque deletion_queue;
};


struct lre_draw_property {
	std::vector<VkFormat> color_format_array;
	VkFormat depth_format;
	VkFormat stencil_format;
};


struct lre_pipeline_registry {
	std::unordered_map<std::string, lvk::config::graphics_pipeline> graphics_pipeline_registry;
	std::unordered_map<std::string, lvk::config::compute_pipeline> compute_pipeline_registry;
};


struct Vertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
};

// holds the resources needed for a mesh
struct GPUMeshBuffers {
    lvk_buffer index_buffer;
    lvk_buffer vertex_buffer;
    VkDeviceAddress vertex_buffer_address;
};


struct GeoSurface {
    uint32_t start_index;
    uint32_t count;
};

struct MeshAsset {
    std::string name;

    std::vector<GeoSurface> surfaces;
    GPUMeshBuffers mesh_buffers;
};

struct GPUDrawPushConstants {
    glm::mat4 world_matrix;
    VkDeviceAddress vertex_buffer;
};

struct GPUSceneData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewproj;
    glm::vec4 ambientColor;
    glm::vec4 sunlightDirection; // w for sun power
    glm::vec4 sunlightColor;
};


namespace lucy {
	class renderer {
		lre_draw_property draw_property = {};
		lre_frame frame_array[FRAMES_IN_FLIGHT] = {};

		std::vector<std::shared_ptr<MeshAsset>> test_meshes;

		lvk_instance instance;
		lvk_device device;
		lvk_swapchain swapchain;
		
		
		// ----------------------------------------------

		GPUSceneData scene_data;

		lvk_descriptor_set_layout gpu_descriptor_set_layout;
		lvk_descriptor_set_layout single_image_descriptor_set_layout;
		
		lvk_buffer gpu_scene_data_buffer;

		GPUMeshBuffers rectangle;
		lvk_pipeline mesh_pipeline;
		lvk_pipeline_layout mesh_pipeline_layout;
		
		lvk_image white_image;
		lvk_image black_image;
		lvk_image grey_image;
		lvk_image error_checkerboard_image;
		lvk_image_view error_checkerboard_image_view;

		lvk_sampler default_sampler_linear;
		lvk_sampler default_sampler_nearest;

		void init_pipeline();
		GPUMeshBuffers init_sample_rectangle();

		// ----------------------------------------------
	
		lvk_deletor_deque deletor;


		VkClearValue clear_value[2] = {
			{
				.color = {
					{ 0.0f, 0.0f, 0, 0.0f }
				}
			},
			{
				.depthStencil = {
					.depth = 1.0f
				}
			},
		};
		
		
		struct {
			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 model;
		} mvp;

		SDL_Window* sdl_window = nullptr;

		GPUMeshBuffers upload_mesh(const std::span<Vertex>& vertices, const std::span<uint32_t>& indices) const;


		lre_frame create_frame(lvk_command_pool&);
		void destroy_frame_images(lre_frame&);
		

		void init_imgui(SDL_Window* sdl_window);
		void draw_imgui(lre_frame& frame);

		void draw_background(lre_frame& frame);
		void draw_geometry(lre_frame& frame);
		
		bool resize_requested = false;

		void record(lre_frame& frame);
		void submit(const lre_frame& frame);

	public:
		void init(SDL_Window* window);

		renderer();

		void set_projection(const glm::mat4& projection);
		void set_view(const glm::mat4& view);
		void set_model(const glm::mat4& model);

		void update(const bool& is_resized);

		void fn_imgui(const std::function<void(lre_frame&, lvk_device&)>&&);
		void fn_record(const std::function<void(lre_frame&, lvk_device&)>&&);
		void fn_(const std::function<void(lre_frame&, lvk_device&)>&&);
		
		std::optional<std::vector<std::shared_ptr<MeshAsset>>> load_GLTF(std::filesystem::path filePath);
		
		void destroy();
	};
}