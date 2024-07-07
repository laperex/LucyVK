#pragma once

#include "lucylm/math.hpp"

#include "lucyvk/mesh.h"
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

namespace lucy {
	class renderer {
		lvk_command_pool main_command_pool;
		lre_frame frame_array[FRAMES_IN_FLIGHT];

		lvk_instance instance;
		lvk_device device;
		lvk_render_pass render_pass;
		lvk_swapchain swapchain;

		
		lvk_descriptor_set_layout descriptor_set_layout;
		lvk_descriptor_pool descriptor_pool;


		// ----------------------------------------------
		
		lvk_pipeline graphics_pipeline;
		lvk_pipeline_layout graphics_pipeline_layout;
		lvk_descriptor_set descriptor_ubo;
		
		lvk::mesh mesh;
		
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
		
		void upload_mesh(lvk::mesh& mesh);
		
		void texture_pipeline_init();
		void descriptor_set_init();
		
		bool resize_requested = false;

	public:
		lvk_image load_image_2D(const char* filename);

		void init(SDL_Window* window);

		renderer();

		void record(lre_frame& frame);
		void submit(const lre_frame& frame);
		
		void set_projection(const glm::mat4& projection);
		void set_view(const glm::mat4& view);
		void set_model(const glm::mat4& model);

		void begin();
		void end();

		void update(const bool& is_resized);
		
		void destroy();
	};
}