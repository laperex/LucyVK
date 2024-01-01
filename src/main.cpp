
#include "lucyvk/vk_instance.h"
#include "lucyvk/vk_physical_device.h"
#include "lucyvk/vk_device.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "Camera.h"
#include "Mesh.h"
#include "lucyvk/vk_config.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_info.h"

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

typedef uint32_t lve_vertex;

#define LVE_CHUNK_SIZE (32)

#define LVE_VERTEX(x, y, z, n, u, v) ((x * LVE_CHUNK_SIZE * LVE_CHUNK_SIZE) + y * LVE_CHUNK_SIZE + z) & 0x7fff

// static lvk::vertex_input_description lve_vertex_description() {
// 	lvk::vertex_input_description description;

// 	//we will have just 1 vertex buffer binding, with a per-vertex rate
// 	VkVertexInputBindingDescription mainBinding = {};
// 	mainBinding.binding = 0;
// 	mainBinding.stride = sizeof(lve_vertex);
// 	mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

// 	description.bindings.push_back(mainBinding);

// 	//Position will be stored at Location 0
// 	VkVertexInputAttributeDescription positionAttribute = {};
// 	positionAttribute.binding = 0;
// 	positionAttribute.location = 0;
// 	positionAttribute.format = VK_FORMAT_R32_UINT;
// 	positionAttribute.offset = 0;

// 	description.attributes.push_back(positionAttribute);

// 	return description;
// }

std::vector<lve_vertex> lve_vertex_triangle = {
	LVE_VERTEX(0, 0, 0, 1, 0, 0),
	LVE_VERTEX(0, 0, 0, 1, 1, 1),
};

lucy::Mesh load_triangle_mesh() {
	lucy::Mesh triangle_mesh;
	
	triangle_mesh._vertices.resize(3);

	//vertex positions
	triangle_mesh._vertices[0].position = { 1.f, 1.f, 0.0f };
	triangle_mesh._vertices[1].position = {-1.f, 1.f, 0.0f };
	triangle_mesh._vertices[2].position = { 0.f,-1.f, 0.0f };

	//vertex colors, all green
	triangle_mesh._vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
	triangle_mesh._vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
	triangle_mesh._vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green
	
	return triangle_mesh;
}

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
	
	// lvk_buffer camera_buffer;

	// lvk_descriptor_set camera_descriptor;
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

	auto instance = lvk_init_instance(&instance_config, window.sdl_window);
	auto physical_device = instance.init_physical_device();
	auto device = physical_device.init_device({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME });
	auto allocator = device.init_allocator();

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

	auto swapchain = device.init_swapchain(window.size.x, window.size.y, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	});

	// auto* depth_image_views = new lvk_image_view[swapchain._image_count];
	// auto* depth_images = new lvk_image[swapchain._image_count];
	// auto* framebuffers = new lvk_framebuffer[swapchain._image_count];

	// auto render_pass = device.init_default_render_pass(VK_FORMAT_R16G16B16A16_SFLOAT);

	// for (int i = 0; i < swapchain._image_count; i++) {
	// 	depth_images[i] = allocator.init_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
	// 	depth_image_views[i] = depth_images[i].init_image_view(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D);

	// 	framebuffers[i] = render_pass.init_framebuffer(swapchain._extent, { swapchain._image_views[i], depth_image_views[i]._image_view });
	// }

	//* ---------------> PIPELINE INIT

	lvk_descriptor_pool descriptor_pool = {};
	{
		struct {
			VkDescriptorType type;
			float ratio;
		} pool_size_ratios[] = {
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 }
		};

		VkDescriptorPoolSize descriptor_pool_sizes[std::size(pool_size_ratios)];
		uint32_t descriptor_set_max_size = 10;
		
		for (int i = 0; i < sizeof(pool_size_ratios); i++) {
			descriptor_pool_sizes[i] = {
				.type = pool_size_ratios[i].type,
				.descriptorCount = static_cast<uint32_t>(pool_size_ratios[i].ratio * descriptor_set_max_size)
			};
		}

		descriptor_pool = device.init_descriptor_pool(descriptor_set_max_size, descriptor_pool_sizes);
	}
	
	lvk_descriptor_set_layout compute_descriptor_set_layout = device.init_descriptor_set_layout({
		lvk::descriptor_set_layout_binding(0, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
		// lvk::descriptor_set_layout_binding(1, VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
	});

	lvk_pipeline_layout compute_pipeline_layout = device.init_pipeline_layout({
		compute_descriptor_set_layout._descriptor_set_layout
	});

	lvk_shader_module compute_shader = device.init_shader_module(VK_SHADER_STAGE_COMPUTE_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/gradient.comp.spv");
	
	lvk_pipeline compute_pipeline = compute_pipeline_layout.init_compute_pipeline(lvk::info::shader_stage(&compute_shader));

	lvk_shader_module vertex_shader = device.init_shader_module(VK_SHADER_STAGE_VERTEX_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/mesh.vert.spv");
	lvk_shader_module fragment_shader = device.init_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/colored_triangle.frag.spv");
	
	auto vertex_layout = lucy::Vertex::get_vertex_description();
	
	lvk_descriptor_set_layout graphics_descriptor_set_layout = device.init_descriptor_set_layout({
		lvk::descriptor_set_layout_binding(1, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
	});
	
	lvk_pipeline_layout graphics_pipeline_layout = device.init_pipeline_layout({
		graphics_descriptor_set_layout._descriptor_set_layout
	});

	lvk_pipeline graphics_pipeline = {};
	{
		lvk::config::graphics_pipeline config = {
			.shader_stage_array = {
				lvk::info::shader_stage(&vertex_shader, nullptr),
				lvk::info::shader_stage(&fragment_shader, nullptr),
			},

			.vertex_input_state = lvk::info::vertex_input_state(
				{
					{
						.binding = 0,
						.stride = sizeof(lucy::Vertex),
						.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
					}
				},
				{
					{
						.location = 0,
						.binding = 0,
						.format = VK_FORMAT_R32G32B32_SFLOAT,
						.offset = offsetof(lucy::Vertex, position),
					},
					{
						.location = 1,
						.binding = 0,
						.format = VK_FORMAT_R32G32B32_SFLOAT,
						.offset = offsetof(lucy::Vertex, normal),
					},
					{
						.location = 2,
						.binding = 0,
						.format = VK_FORMAT_R32G32B32_SFLOAT,
						.offset = offsetof(lucy::Vertex, color),
					}
				}
			),
			.input_assembly_state = lvk::info::input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, false),
			.rasterization_state = lvk::info::rasterization_state(VK_POLYGON_MODE_FILL),
			.multisample_state = lvk::info::multisample_state(),
			.depth_stencil_state = lvk::info::depth_stencil_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL),
			.color_blend_attachment = lvk::color_blend_attachment(),
			
			.viewport = {
				.x = 0.0f,
				.y = 0.0f,
				.width = (float)window.size.x,
				.height = (float)window.size.y,
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			},

			.scissor = {
				.offset = { 0, 0 },
				.extent = { static_cast<uint32_t>(window.size.x), static_cast<uint32_t>(window.size.y) }
			}
		};

		graphics_pipeline = graphics_pipeline_layout.init_graphics_pipeline(&config);
	}

	lucy::Mesh monkey_mesh;
	monkey_mesh.load_obj("/home/laperex/Programming/C++/LucyVK/src/assets/monkey.obj");
	monkey_mesh.vertex_buffer = allocator.init_vertex_buffer(monkey_mesh._vertices.data(), monkey_mesh._vertices.size() * sizeof(monkey_mesh._vertices[0]));

	VkImageUsageFlags draw_image_usages = 
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_STORAGE_BIT |
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	struct {
		lvk_image image;
		lvk_image_view image_view;
		VkExtent2D extent;

		lvk_descriptor_set graphics_descriptor;
		lvk_descriptor_set compute_descriptor;

		lvk_buffer uniform_buffer;

		lvk_image depth_image;
		lvk_image_view depth_image_view;
		// lvk_framebuffer framebuffer;
	} draw;
	
	draw.image = allocator.init_image(VK_FORMAT_R16G16B16A16_SFLOAT, draw_image_usages, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
 	draw.image_view = draw.image.init_image_view(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
	draw.graphics_descriptor = descriptor_pool.init_descriptor_set(&graphics_descriptor_set_layout);
	draw.compute_descriptor = descriptor_pool.init_descriptor_set(&compute_descriptor_set_layout);
	
	draw.uniform_buffer = allocator.init_uniform_buffer<mvp_matrix>();

	// frame_array[i].camera_descriptor.update(&frame_array[i].camera_buffer);
	draw.compute_descriptor.update(0, &draw.image_view, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	// draw.compute_descriptor.update(1, &draw.uniform_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	draw.graphics_descriptor.update(1, &draw.uniform_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	
	draw.depth_image = allocator.init_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
	draw.depth_image_view = draw.depth_image.init_image_view(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D);

	// draw.framebuffer = render_pass.init_framebuffer(swapchain._extent, { draw.image_view._image_view, draw.depth_image_view._image_view });
	
	lucy::Camera camera = {};
	camera.width = window.size.x;
	camera.height = window.size.y;

	VkClearValue clear_value[] = {
		{
			.color = { { 0.0f, 0.0f, 0, 0.0f } }
		},
		{
			.depthStencil = {
				.depth = 1.0f
			}
		},
	};

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

		// TODO! IMPLEMENT COMPUTE SHADERS

		{
			{
				auto& current_frame = frame_array[frame_number % FRAMES_IN_FLIGHT];
				auto& cmd = current_frame.command_buffer;

				uint32_t image_index;
				swapchain.acquire_next_image(&image_index, current_frame.present_semaphore._semaphore, VK_NULL_HANDLE);
				current_frame.image_index = image_index;

				draw.extent = { draw.image._extent.width, draw.image._extent.height };				
				
				mvp_matrix mvp = {
					.projection = camera.projection,
					.view = camera.view,
					.model = glm::rotate(glm::mat4(1.0f), glm::radians(frame_number * 0.4f), glm::vec3(0, 1, 0)),
					.color = { 0, 1, 0, 1},
				};

				draw.uniform_buffer.upload(mvp);

				cmd.reset();

				cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
					//make the swapchain image into writeable mode before rendering
				// cmd.begin_render_pass(&draw.framebuffer, VK_SUBPASS_CONTENTS_INLINE, clear_value);

				cmd.transition_image(draw.image._image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
				{
					VkClearColorValue clear_color_value;
					float flash = abs(sin(frame_number / 120.f));
					clear_color_value = { { 0.0f, 0.0f, flash, 1.0f } };

					VkImageSubresourceRange clear_range = lvk::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT);

					//clear image
					// vkCmdClearColorImage(cmd._command_buffer, draw.image._image, VK_IMAGE_LAYOUT_GENERAL, &clear_color_value, 1, &clear_range);
					cmd.bind_pipeline(&compute_pipeline);

					vkCmdBindDescriptorSets(cmd._command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout._pipeline_layout, 0, 1, &draw.compute_descriptor._descriptor_set, 0, VK_NULL_HANDLE);

					vkCmdDispatch(cmd._command_buffer, std::ceil(draw.extent.width / 16.0), std::ceil(draw.extent.height / 16.0), 1);
				}

				cmd.transition_image(draw.image._image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

					VkRenderingAttachmentInfo depth_attachment = {
						.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
						.imageView = draw.depth_image_view._image_view,
						.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
						.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
						.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
						.clearValue = {
							.depthStencil = {
								.depth = 1.0f
							}
						},
					};
				{
					VkRenderingAttachmentInfo color_attachment = {
						.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
						.imageView = draw.image_view._image_view,
						.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
						.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
						.clearValue = {
							.color = { 0, 0, 0, 0 }
						}
					};

					
					// VkPipelineRenderingCreateInfo

					VkRenderingInfoKHR render_info = {
						.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
						.renderArea = {
							.offset = { 0, 0 },
							.extent = swapchain._extent,
						},
						.layerCount = 1,
						
						.colorAttachmentCount = 1,
						.pColorAttachments = &color_attachment,

						.pDepthAttachment = &depth_attachment,
						// .viewMask
					};

					vkCmdBeginRendering(cmd._command_buffer, &render_info);

					cmd.bind_vertex_buffers({ monkey_mesh.vertex_buffer._buffer }, { 0 });
					vkCmdBindPipeline(cmd._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline._pipeline);
					vkCmdBindDescriptorSets(cmd._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout._pipeline_layout, 0, 1, &draw.graphics_descriptor._descriptor_set, 0, VK_NULL_HANDLE);

					vkCmdDraw(cmd._command_buffer, monkey_mesh._vertices.size(), 1, 0, 0);
					
					vkCmdEndRendering(cmd._command_buffer);
				}

				cmd.transition_image(draw.image._image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

				cmd.transition_image(swapchain._images[image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				
				cmd.blit_image_to_image(draw.image._image, swapchain._images[image_index], draw.extent, swapchain._extent);
				
				cmd.transition_image(swapchain._images[image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

				// cmd.begin_render_pass(&test_framebuffer, VK_SUBPASS_CONTENTS_INLINE, VK_NULL_HANDLE, 0);

				// vkCmdBindPipeline(cmd._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline._pipeline);
				
				// cmd.bind_vertex_buffers({ monkey_mesh.vertex_buffer._buffer }, { 0 });

				// glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(frame_number * 0.4f), glm::vec3(0, 1, 0));

				// mvp_matrix mvp = {
				// 	.projection = camera.projection,
				// 	.view = camera.view,
				// 	.model = glm::rotate(glm::mat4(1.0f), glm::radians(frame_number * 0.4f), glm::vec3(0, 1, 0)),
				// };

				// glm::mat4 mesh_matrix = camera.projection * camera.view * model;

				// lucy::MeshPushConstants constants;
				// constants.render_matrix = mesh_matrix;
				// constants.offset = { 0, 0, 0, 0};

				// current_frame.camera_buffer.upload(mvp);

				// // vkCmdPushConstants(cmd._command_buffer, pipeline_layout._pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(lucy::MeshPushConstants), &constants);
				// vkCmdBindDescriptorSets(cmd._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout._pipeline_layout, 0, 1, &current_frame.global_descriptor._descriptor_set, 0, nullptr);

				// vkCmdDraw(cmd._command_buffer, monkey_mesh._vertices.size(), 1, 0, 0);

				// cmd.end_render_pass();

				cmd.end();
				// exit(0);
			}

			if (frame_number > 0) {
				auto& frame = frame_array[(frame_number - 1) % FRAMES_IN_FLIGHT];
				
				VkCommandBufferSubmitInfo cmd_info = frame.command_buffer.submit_info();
	
				VkSemaphoreSubmitInfo wait_info = lvk::info::semaphore_submit(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, frame.present_semaphore._semaphore);
				VkSemaphoreSubmitInfo signal_info = lvk::info::semaphore_submit(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, frame.render_semaphore._semaphore);
				
				// VkSubmitInfo2 submit = lvk::info::submit2(&cmdinfo, &signalInfo, &waitInfo);
				
				if (device.submit2({ lvk::info::submit2(&cmd_info, &signal_info, &wait_info) }, &frame.render_fence) != VK_SUCCESS) {
					throw std::runtime_error("Submit2 failed!");
				}
				
				frame.render_fence.wait();
				frame.render_fence.reset();
				
				// VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

				// VkSubmitInfo submit = {
				// 	.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
					
				// 	.waitSemaphoreCount = 1,
				// 	.pWaitSemaphores = &prev_frame.present_semaphore._semaphore,

				// 	.pWaitDstStageMask = &waitStage,

				// 	.commandBufferCount = 1,
				// 	.pCommandBuffers = &prev_frame.command_buffer._command_buffer,

				// 	.signalSemaphoreCount = 1,
				// 	.pSignalSemaphores = &prev_frame.render_semaphore._semaphore,
				// };

				// device.submit(&submit, 1, &prev_frame.render_fence);
				
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

		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
		frame_number += 1;
	}
	
	device.wait_idle();

	window.Destroy();
}