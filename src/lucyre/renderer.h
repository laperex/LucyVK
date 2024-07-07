#pragma once

#include "lucylm/math.hpp"

#include "lucyre/mesh.h"
#include "lucyvk/command.h"
#include "lucyvk/device.h"
#include "lucyvk/handles.h"
#include "lucyvk/instance.h"
#include "lucyvk/functions.h"
// #include "lucyvk/memory.h"
// #include "lucyvk/synchronization.h"
// #include "lucyvk/types.h"
#include <vector>
// #include "lucyvk/render_pass.h"
// #include "lucyvk/swapchain.h"

#define FRAMES_IN_FLIGHT 2


struct lre_frame {
	lvk_fence render_fence;

	lvk_semaphore present_semaphore;
	lvk_semaphore render_semaphore;

	lvk_command_buffer command_buffer;

	uint32_t image_index;
};


namespace lre {
	class renderer {
		lvk_command_pool main_command_pool;
		lre_frame frame_array[FRAMES_IN_FLIGHT];

		lvk_instance instance;
		lvk_device device;

		lvk_swapchain swapchain;
		// std::vector<lvk_framebuffer> framebuffer_array;

		// lvk_image depth_image;
		// lvk_image_view depth_image_view;

		lvk_descriptor_pool descriptor_pool;

		lvk_pipeline graphics_pipeline;
		lvk_pipeline_layout graphics_pipeline_layout;

		lvk_descriptor_set descriptor_ubo;
		lvk_descriptor_set_layout descriptor_set_layout;
		
		// lvk_image load_image;
		// lvk_image_view load_image_view;


		// lvk_sampler sampler;


		lvk_render_pass render_pass;
		
		lvk_buffer mvp_uniform_buffer;
		
		Mesh mesh;
		
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
		
		void upload_mesh(Mesh& mesh);
		
		void texture_pipeline_init();
		void descriptor_set_init();
		
		bool resize_requested = false;

	public:
		lvk_image load_image_2D(const char* filename);

		renderer();
	
		void init(SDL_Window* window);


		void record(lre_frame& frame);
		void submit(const lre_frame& frame);

		void begin();
		void end();

		void update(const bool& is_resized);
		
		void destroy();
	};
}