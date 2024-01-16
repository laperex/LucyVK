#pragma once

#include "lucyvk/vk_render_pass.h"
#include "sdl_window.h"
#include <lucyvk/lucyvk.h>

#define FRAMES_IN_FLIGHT 2

namespace lucy {
	struct pipeline_config {
		
	};
	
	class vk_renderer {
		struct {
			lvk_fence render_fence;
			lvk_semaphore present_semaphore;
			lvk_semaphore render_semaphore;

			lvk_command_pool command_pool;
			lvk_command_buffer command_buffer;

			uint32_t image_index;
		} frame_array[FRAMES_IN_FLIGHT];


		lvk_instance instance;
		lvk_physical_device physical_device;
		lvk_device device;
		lvk_allocator allocator;

		lvk_swapchain swapchain;
		
		lvk_descriptor_pool descriptor_pool;
		
		lvk_pipeline graphics_pipeline;
		lvk_pipeline_layout graphics_pipeline_layout;
		lvk_descriptor_set graphics_descriptor;
		lvk_descriptor_set_layout graphics_descriptor_set_layout;
		
		lvk_render_pass render_pass;
		lvk_framebuffer* framebuffer_array;

		lvk_image depth_image;
		lvk_image_view depth_image_view;
		
		lvk_buffer uniform_buffer;
		
		
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

		void init_frame_data();
		void init_swapchain(const glm::ivec2 size);
		void init_descriptor_pool();
		
		void init_render_pass();
		void init_pipeline();

	public:
		void initialization(sdl_window* window);
		void record(uint32_t frame_number);
		void submit(uint32_t frame_number);
		
		void destruction();
	};
}