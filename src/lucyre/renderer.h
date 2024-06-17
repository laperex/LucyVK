#pragma once

#include "lucylm/math.hpp"

#include "lucyre/mesh.h"
#include "lucyvk/command.h"
#include "lucyvk/device.h"
#include "lucyvk/instance.h"
#include "lucyvk/functions.h"
#include "lucyvk/memory.h"
// #include "lucyvk/synchronization.h"
#include "lucyvk/types.h"
// #include "lucyvk/render_pass.h"
// #include "lucyvk/swapchain.h"

#define FRAMES_IN_FLIGHT 2

namespace lre {
	class renderer {
		struct {
			lvk_fence render_fence;

			lvk_semaphore present_semaphore;
			lvk_semaphore render_semaphore;

			lvk_command_buffer command_buffer;

			uint32_t image_index;
		} frame_array[FRAMES_IN_FLIGHT];


		lvk_command_pool command_pool;
		
		lvk_immediate_command immediate_command;

		lvk_instance instance;
		// lvk_physical_device physical_device;
		lvk_device device;
		lvk_allocator allocator;

		lvk_swapchain swapchain;
		
		lvk_descriptor_pool descriptor_pool;
		
		lvk_pipeline graphics_pipeline;
		lvk_pipeline_layout graphics_pipeline_layout;
		lvk_descriptor_set descriptor;
		lvk_descriptor_set_layout descriptor_set_layout;
		
		// lvk_pipeline compute_pipeline;
		// lvk_pipeline_layout compute_pipeline_layout;
		// // lvk_descriptor_set compute_descriptor;
		// // lvk_descriptor_set_layout compute_descriptor_set_layout;
		
		lvk_render_pass render_pass;
		lvk_framebuffer* framebuffer_array;

		lvk_image depth_image;
		lvk_image_view depth_image_view;
		
		lvk_buffer mvp_uniform_buffer;
		lvk_image_view compute_image_view;
		// lvk_image compute_image;
		
		
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
		
		SDL_Window* sdl_window = nullptr;

		void init_frame_data();
		void init_upload_mesh();
		void init_swapchain(const glm::ivec2 size);
		void init_descriptor_pool();
		
		void init_render_pass();
		void init_pipeline();
		
		void upload_mesh(Mesh& mesh);
		bool load_image_from_file(const char* filename, lvk_image& image);

	public:
		renderer();
	
		void initialization(SDL_Window*);
		
		void record(uint32_t frame_number);
		void submit(uint32_t frame_number);
		
		void update();
		
		void destroy();
	};
}