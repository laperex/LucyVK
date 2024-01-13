

#include "lucyvk/vk_config.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_info.h"
#include "lucyvk/vk_instance.h"
#include "lucyvk/vk_render_pass.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "stb_image.h"

#include "Camera.h"
#include "Mesh.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>
#include <Events.h>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>
#include <Window.h>
#include <glm/glm.hpp>

#include <glm/gtx/transform.hpp>

#include <util/logger.h>

#include "lucyvk/lucyvk.h"

#include <glm/gtc/noise.hpp>

struct mvp_matrix {
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
	glm::vec4 color;
};

struct Frame {
	lvk_fence render_fence;
	lvk_semaphore present_semaphore;
	lvk_semaphore render_semaphore;

	lvk_command_pool command_pool;
	lvk_command_buffer command_buffer;
	
	uint32_t image_index;
};

static constexpr const int FRAMES_IN_FLIGHT = 2;

int main(int count, char** args) {
	lucy::Window window = {};
	window.InitWindow();

	lvk::config::instance instance_config = {
		.name = "Lucy Framework v7",
		.enable_validation_layers = true
	};
	
	//* ---------------> INSTANCE INIT

	lvk_instance instance = lvk_init_instance(&instance_config, window.sdl_window);
	lvk_physical_device physical_device = instance.init_physical_device();
	lvk_device device = physical_device.init_device({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME });
	lvk_allocator allocator = device.init_allocator();

	//* ---------------> COMMAND POOL INIT

	Frame frame_array[FRAMES_IN_FLIGHT];

	for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
		frame_array[i].command_pool = device.init_command_pool();
		frame_array[i].command_buffer = frame_array[i].command_pool.init_command_buffer();

		frame_array[i].render_fence = device.init_fence();

		frame_array[i].render_semaphore = device.init_semaphore();
		frame_array[i].present_semaphore = device.init_semaphore();

		// frame_array[i].camera_buffer = allocator.init_uniform_buffer<mvp_matrix>();
	
		// frame_array[i].camera_descriptor = descriptor_pool.init_descriptor_set(&descriptor_set_layout);

		// frame_array[i].camera_descriptor.update(&frame_array[i].camera_buffer);
	}
	
	//* ---------------> SWAPCHAIN INIT

	lvk_swapchain swapchain = device.init_swapchain(window.size.x, window.size.y, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	});

	//* ---------------> PIPELINE INIT

	uint32_t descriptor_set_max_size = 10;
	VkDescriptorPoolSize descriptor_pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }
	};

	lvk_descriptor_pool descriptor_pool = device.init_descriptor_pool(descriptor_set_max_size, descriptor_pool_sizes);
	
	lvk_descriptor_set_layout compute_descriptor_set_layout = device.init_descriptor_set_layout({
		lvk::descriptor_set_layout_binding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
		lvk::descriptor_set_layout_binding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
	});

	lvk_pipeline_layout compute_pipeline_layout = device.init_pipeline_layout({
		compute_descriptor_set_layout._descriptor_set_layout
	});

	lvk_shader_module compute_shader = device.init_shader_module(VK_SHADER_STAGE_COMPUTE_BIT, "./shaders/raymarch.comp.spv");
	
	lvk_pipeline compute_pipeline = compute_pipeline_layout.init_compute_pipeline(lvk::info::shader_stage(&compute_shader));

	lvk_shader_module vertex_shader = device.init_shader_module(VK_SHADER_STAGE_VERTEX_BIT, "./shaders/terrain.vert.spv");
	lvk_shader_module fragment_shader = device.init_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, "./shaders/terrain.frag.spv");
	
	// auto vertex_layout = lucy::Vertex::get_vertex_description();
	
	lvk_descriptor_set_layout graphics_descriptor_set_layout = device.init_descriptor_set_layout({
		lvk::descriptor_set_layout_binding(1, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
	});
	
	VkViewport viewport[] = {{
		.x = 0.0f,
		.y = 0.0f,
		.width = window.size.x,
		.height = window.size.y,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	}};
	
	VkRect2D scissor[] = {
		{
			.offset = { 0, 0 },
			.extent = { static_cast<uint32_t>(window.size.x), static_cast<uint32_t>(window.size.y) }
		}
	};

	lvk::config::graphics_pipeline config = {
		.shader_stage_array = {
			lvk::info::shader_stage(&vertex_shader),
			lvk::info::shader_stage(&fragment_shader),
		},

		.vertex_input_state = lvk::info::vertex_input_state({
			{
				.binding = 0,
				.stride = sizeof(glm::vec3),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
			}
		},{
			{
				.location = 0,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = 0,
			}
		}),

		.input_assembly_state = lvk::info::input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false),
		
		.viewport_state = lvk::info::viewport_state({
			{
				.x = 0.0f,
				.y = 0.0f,
				.width = window.size.x,
				.height = window.size.y,
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			},
		},{
			{
				.offset = { 0, 0 },
				.extent = { static_cast<uint32_t>(window.size.x), static_cast<uint32_t>(window.size.y) }
			}
		}),
		
		.rasterization_state = lvk::info::rasterization_state(VK_POLYGON_MODE_LINE),
		.multisample_state = lvk::info::multisample_state(),
		.depth_stencil_state = lvk::info::depth_stencil_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL),
		
		.color_blend_state = lvk::info::color_blend_state({
			{
				.blendEnable = VK_FALSE,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}
		}),
		
		// .dynamic_rendering = lvk::info::rendering(VK_FORMAT_D32_SFLOAT)
	};
	// VkPipelineViewportStateCreateInfo viewport_info = ;
	
	config.viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		
		.viewportCount = 1,
		.pViewports = viewport,
		
		.scissorCount = 1,
		.pScissors = scissor
	};
	
	lvk_descriptor_set graphics_descriptor = descriptor_pool.init_descriptor_set(&graphics_descriptor_set_layout);

	lvk_image depth_image = allocator.init_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
	lvk_image_view depth_image_view = depth_image.init_image_view(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D);

	VkClearValue clear_value[] = {
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

	lvk_buffer uniform_buffer = allocator.init_uniform_buffer<mvp_matrix>();
	lvk_render_pass render_pass = device.init_default_render_pass(swapchain._surface_format.format);
	lvk_framebuffer* framebuffer_array = new lvk_framebuffer[swapchain._image_count];
	
	graphics_descriptor.update(1, &uniform_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	for (int i = 0; i < swapchain._image_count; i++) {
		framebuffer_array[i] = render_pass.init_framebuffer(swapchain._extent, { swapchain._image_views[i], depth_image_view._image_view });
	}

	lvk_pipeline_layout graphics_pipeline_layout = device.init_pipeline_layout({
		graphics_descriptor_set_layout._descriptor_set_layout
	});
	
	lvk_pipeline graphics_pipeline = graphics_pipeline_layout.init_graphics_pipeline(&config, &render_pass);

	struct {
		std::vector<glm::vec3> vertices;
		lvk_buffer vertex_buffer;
	} terrain;

	for (int x = 0; x < 100; x++) {
		for (int z = 0; z < 100; z++) {
			terrain.vertices.push_back({ 1 + x, (((glm::perlin(glm::vec2{(1 + x) / 160.0, (1 + z) / 160.0})) + 1) / 2)*(32) - 16,  1 + z});
			terrain.vertices.push_back({ 0 + x, (((glm::perlin(glm::vec2{(0 + x) / 160.0, (1 + z) / 160.0})) + 1) / 2)*(32) - 16,  1 + z});
			terrain.vertices.push_back({ 0 + x, (((glm::perlin(glm::vec2{(0 + x) / 160.0, (0 + z) / 160.0})) + 1) / 2)*(32) - 16,  0 + z});

			terrain.vertices.push_back({ 1 + x, (((glm::perlin(glm::vec2{(1 + x) / 160.0, (1 + z) / 160.0})) + 1) / 2)*(32) - 16,  1 + z});
			terrain.vertices.push_back({ 0 + x, (((glm::perlin(glm::vec2{(0 + x) / 160.0, (0 + z) / 160.0})) + 1) / 2)*(32) - 16,  0 + z});
			terrain.vertices.push_back({ 1 + x, (((glm::perlin(glm::vec2{(1 + x) / 160.0, (0 + z) / 160.0})) + 1) / 2)*(32) - 16,  0 + z});
		}
	}

	terrain.vertices.push_back({ 1, 0,  1 });
	terrain.vertices.push_back({ 0, 0,  1 });
	terrain.vertices.push_back({ 0, 0,  0 });

	terrain.vertices.push_back({ 1, 0,  1 });
	terrain.vertices.push_back({ 0, 0,  0 });
	terrain.vertices.push_back({ 1, 0,  0 });
	
	dloggln((((glm::perlin(glm::vec2{(0) / 160.0, (0) / 160.0})) + 1) / 2)*(32));
	dloggln((((glm::perlin(glm::vec2{(0) / 160.0, (1) / 160.0})) + 1) / 2)*(32));
	dloggln((((glm::perlin(glm::vec2{(0) / 160.0, (0) / 160.0})) + 1) / 2)*(32));
	dloggln((((glm::perlin(glm::vec2{(1) / 160.0, (1) / 160.0})) + 1) / 2)*(32));
	dloggln((((glm::perlin(glm::vec2{(0) / 160.0, (0) / 160.0})) + 1) / 2)*(32));
	// float value = ;
	// value = ((glm::perlin(glm::vec2{(0) / 160.0, (0) / 160.0})) + 1);
	// value *= 128 + 128;

	terrain.vertex_buffer = allocator.init_vertex_buffer(terrain.vertices.data(), terrain.vertices.size() * sizeof(terrain.vertices[0]));
	dloggln(terrain.vertices.size() * sizeof(terrain.vertices[0]));
	
	// terrain.vertex_buffer

	lucy::Camera camera = {};
	camera.width = window.size.x;
	camera.height = window.size.y;
	
	double dt = 0;
	uint32_t frame_number = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		camera.Update(dt);

		// TODO: Shader Module creation/deletion/query management
		// TODO: vk_shaders.h
		// TODO: maybe restructure init_ based creation
		// TODO: Remove deletion_queue and find a better approach
		// TODO: Better approach for PhysicalDevice selection and Initialization
		// TODO: Add delta time Macros
		// TODO: multi_init feature for initialization of multiple vulkan types

		// // TODO! IMPLEMENT COMPUTE SHADERS

		{
			{
				auto& current_frame = frame_array[frame_number % FRAMES_IN_FLIGHT];
				auto& cmd = current_frame.command_buffer;

				uint32_t image_index;
				swapchain.acquire_next_image(&image_index, current_frame.present_semaphore._semaphore, VK_NULL_HANDLE);
				current_frame.image_index = image_index;

				// draw.extent = *(const VkExtent2D*)&draw.image._extent;

				mvp_matrix mvp = {
					.projection = camera.projection,
					.view = camera.view,
					.model = glm::mat4(1.0f),	//glm::rotate(glm::mat4(1.0f), glm::radians(frame_number * 0.4f), glm::vec3(0, 1, 0)),
					.color = { 0, 1, 0, 1},
				};

				uniform_buffer.upload(mvp);

				cmd.reset();

				cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
				
				VkRenderPassBeginInfo render_pass_begin_info = {
					.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
					.renderPass = render_pass._render_pass,
					.framebuffer = framebuffer_array[image_index]._framebuffer,
					.renderArea = {
						.offset = { 0, 0 },
						.extent = swapchain._extent,
					},
					.clearValueCount = std::size(clear_value),
					.pClearValues = clear_value
				};
				
				cmd.begin_render_pass(&render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
				
				cmd.bind_vertex_buffers({ terrain.vertex_buffer._buffer }, { 0 });
				cmd.bind_pipeline(&graphics_pipeline);

				vkCmdBindDescriptorSets(cmd._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout._pipeline_layout, 0, 1, &graphics_descriptor._descriptor_set, 0, VK_NULL_HANDLE);

				vkCmdDraw(cmd._command_buffer, terrain.vertices.size(), 1, 0, 0);
			
				cmd.end_render_pass();

				cmd.end();
			}

			if (frame_number > 0) {
				auto& frame = frame_array[(frame_number - 1) % FRAMES_IN_FLIGHT];

				VkPipelineStageFlags wait_dest = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

				VkSubmitInfo submit_info = {
					.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					
					.waitSemaphoreCount = 1,
					.pWaitSemaphores = &frame.present_semaphore._semaphore,
					
					.pWaitDstStageMask = &wait_dest,
					
					.commandBufferCount = 1,
					.pCommandBuffers = &frame.command_buffer._command_buffer,
					
					.signalSemaphoreCount = 1,
					.pSignalSemaphores = &frame.render_semaphore._semaphore,
				};

				device.submit(&submit_info, 1, &frame.render_fence);

				frame.render_fence.wait();
				frame.render_fence.reset();

				VkPresentInfoKHR presentInfo = {
					.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
					
					.waitSemaphoreCount = 1,
					.pWaitSemaphores = &frame.render_semaphore._semaphore,
					
					.swapchainCount = 1,
					.pSwapchains = &swapchain._swapchain,
					
					.pImageIndices = &frame.image_index,
				};

				device.present(&presentInfo);
			}
		}

		frame_number += 1;
		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
	
	device.wait_idle();

	window.Destroy();
}