#include "lucyvk/functions.h"
#include "lucyvk/create_info.h"
// #include "lucyvk/shaders.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <assimp/Importer.hpp>

#include "lucyio/logger.h"

#include "renderer.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <imgui.h>


lucy::renderer::renderer()
{
	pipeline_manager.device = &device;
}


void lucy::renderer::upload_mesh(lvk::mesh& mesh) {
	mesh.vertex_buffer = device.create_vertex_buffer(mesh.vertices.size(), mesh.vertices.data());
	
}

lvk_image lucy::renderer::load_model(const char* filename) {
	
}

lvk_image lucy::renderer::load_image_2D(const char* filename) {
	int width, height, channels;

	stbi_uc* image_data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if (!image_data) {
		dloggln("Failed to load texture file ", filename);
		return { ._image = VK_NULL_HANDLE };
	}

	VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;
	VkDeviceSize image_size = width * height * 4;
	VkExtent3D image_extent = {
		.width = static_cast<uint32_t>(width),
		.height = static_cast<uint32_t>(height),
		.depth = 1,
	};

	lvk_buffer staging_buffer = this->device.create_staging_buffer(image_size, image_data);

	lvk_image image = this->device.create_image(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY, image_extent, VK_IMAGE_TYPE_2D);

	this->device.imm_submit([&](lvk_command_buffer command_buffer) {
		VkImageMemoryBarrier image_barrier = lvk::info::image_memory_barrier(image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, lvk::info::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT));

		command_buffer.pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, image_barrier);
		command_buffer.copy_buffer_to_image(staging_buffer, image, image._extent);

		image_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		command_buffer.pipeline_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, image_barrier);
	});

	deletor.destroy(staging_buffer);
	stbi_image_free(image_data);

	return image;
}

void lucy::renderer::texture_pipeline_init() {
	lvk_shader_module vertex_shader = device.create_shader_module("./shaders/texture.vert.spv");
	lvk_shader_module fragment_shader = device.create_shader_module("./shaders/texture.frag.spv");

	VkDynamicState state[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	lvk::vertex_input_description vertex_description = lvk::vertex::get_vertex_input_description();
	
	const VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
		.colorAttachmentCount = 1,
		.pColorAttachmentFormats = &swapchain._surface_format.format,

		// .depthAttachmentFormat
	};
	
	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	lvk::config::graphics_pipeline config = {
		.shader_stage_array = {
			lvk::info::shader_stage(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader),
			lvk::info::shader_stage(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader),
		},

		.vertex_input_state = lvk::info::vertex_input_state(vertex_description.bindings, vertex_description.attributes),

		.input_assembly_state = lvk::info::input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),
		
		.rasterization_state = lvk::info::rasterization_state(VK_POLYGON_MODE_FILL),

		.multisample_state = lvk::info::multisample_state(),

		.depth_stencil_state = lvk::info::depth_stencil_state(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL),

		.color_blend_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,

			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY,

			.attachmentCount = 1,
			.pAttachments = &color_blend_attachment_state,
		},

		// TODO: Abstraction [Temporary Structs for Storage only with std::vector]
		// TODO: create_graphics_pipeline with seperate config parameters for below structs
		.viewport_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			
			.viewportCount = 1,
			.scissorCount = 1
		},
		
		.dynamic_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,

			.dynamicStateCount = std::size(state),
			.pDynamicStates = state,
		},
		
		.rendering_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
			
			.pNext = VK_NULL_HANDLE,

			.colorAttachmentCount = 1,
    		.pColorAttachmentFormats = &swapchain._surface_format.format,
			.depthAttachmentFormat = swapchain._depth_image._format,
			// .stencilAttachmentFormat = swapchain._depth_image._format
		}
	};

	graphics_pipeline_layout = deletor.push(device.create_pipeline_layout({ descriptor_set_layout }));
	graphics_pipeline = deletor.push(device.create_graphics_pipeline_dynamic(graphics_pipeline_layout, config));

	deletor.destroy(fragment_shader);
	deletor.destroy(vertex_shader);
}

void lucy::renderer::descriptor_set_init() {
	uint32_t max_descriptor_sets = 100;

	descriptor_pool = deletor.push(device.create_descriptor_pool(max_descriptor_sets, {
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 },
	}));

	// seperate for each shader type
	descriptor_set_layout = deletor.push(
		device.create_descriptor_set_layout({
			lvk::info::descriptor_set_layout_binding(0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
			lvk::info::descriptor_set_layout_binding(1, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
			lvk::info::descriptor_set_layout_binding(2, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1),
		})
	);

	global_descriptor = device.create_descriptor_set(descriptor_pool, descriptor_set_layout);
	// device.create_descriptor_set(descriptor_pool, descriptor_set_layout);
}

void lucy::renderer::init_imgui(SDL_Window* sdl_window) {
	VkDescriptorPoolSize pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};
	
	VkDescriptorPool imgui_descrptor_pool = device.create_descriptor_pool(1000, pool_sizes, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

	ImGui::CreateContext();
	// this initializes imgui for SDL
	ImGui_ImplSDL2_InitForVulkan(sdl_window);

	// this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {
		.Instance = instance._instance,
		.PhysicalDevice = device.get_physical_device(),
		.Device = device.get_logical_device(),
		.Queue = device.get_graphics_queue(),
		.DescriptorPool = imgui_descrptor_pool,
		.MinImageCount = 3,
		.ImageCount = 3,

		//dynamic rendering parameters for imgui to use
		.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
		.UseDynamicRendering = true,

		.PipelineRenderingCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &swapchain._surface_format.format,
		},
	};

	ImGui_ImplVulkan_Init(&init_info);
	ImGui_ImplVulkan_CreateFontsTexture();

	// add the destroy the imgui created structures
	deletor.push_fn([=]() {
		dloggln("IMGUI DESTROYED");

		ImGui_ImplVulkan_Shutdown();
		vkDestroyDescriptorPool(init_info.Device, imgui_descrptor_pool, nullptr);
	});

	dloggln("IMGUI INITIALIZED");
}

void lucy::renderer::init(SDL_Window* window) {
	instance = lvk_instance::initialize("Lucy Framework v17", window, true);
	device = instance.create_device({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME });
	deletor = device.create_deletor();
	
	sdl_window = window;

	main_command_pool = deletor.push(device.create_graphics_command_pool());

	for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
		frame_array[i].command_buffer = deletor.push(device.create_command_buffer(main_command_pool), main_command_pool);
		// deletor.push(frame_array[i].command_buffer)

		frame_array[i].render_fence = deletor.push(device.create_fence());
		frame_array[i].render_semaphore = deletor.push(device.create_semaphore());
		frame_array[i].present_semaphore = deletor.push(device.create_semaphore());
	}
	
	
	mesh.vertices = std::vector<lvk::vertex> {
		{ {  100,  100, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f } },
		{ { -100,  100, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { -100, -100, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } },
		{ {  100, -100, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
	};
	
	mesh.indices = { 0,1,2, 2,3,0 };
	
	mesh.index_buffer = deletor.push(device.create_index_buffer_static(mesh.indices));
	mesh.vertex_buffer = deletor.push(device.create_vertex_buffer(mesh.vertices));


	glm::ivec2 size;
	SDL_GetWindowSize(window, &size.x, &size.y);

	render_pass = deletor.push(device.create_default_render_pass(VK_FORMAT_B8G8R8A8_UNORM));
	swapchain = device.create_swapchain(render_pass, size.x, size.y, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	});

	init_imgui(sdl_window);
	
	descriptor_set_init();

	texture_pipeline_init();

	lvk_image load_image = deletor.push(load_image_2D("/home/laperex/Programming/C++/LucyVK/assets/buff einstein.jpg"));
	lvk_image_view load_image_view = deletor.push(device.create_image_view(load_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT));
	lvk_sampler sampler = deletor.push(device.create_sampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT));

	device.update_descriptor_set(global_descriptor, 2, &load_image_view, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);

	set_model(glm::mat4(1.0f));
}

void lucy::renderer::record(lre_frame& frame) {
	const auto& cmd = frame.command_buffer;

	if (device.swapchain_acquire_next_image(swapchain, &frame.image_index, frame.present_semaphore, VK_NULL_HANDLE) == VK_ERROR_OUT_OF_DATE_KHR) {
		resize_requested = true;
		return;
	}

	// mvp.projection[1][1] *= -1;
	static lvk_buffer mvp_uniform_buffer = {
		._buffer = VK_NULL_HANDLE
	};
	
	if (mvp_uniform_buffer._buffer == VK_NULL_HANDLE) {
		mvp_uniform_buffer = deletor.push(device.create_uniform_buffer<decltype(mvp)>());
		device.update_descriptor_set(global_descriptor, 1, &mvp_uniform_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
	}

	// mvp.projection[1][1] *= -1;

	device.upload(mvp_uniform_buffer, mvp);

	cmd.reset();
	cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	{
		const VkImageMemoryBarrier image_memory_barrier {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,

			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			

			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,

			.image = swapchain._images[frame.image_index],
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		vkCmdPipelineBarrier(cmd._command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	}

	cmd.set_viewport(lvk::info::viewport(0, 0, swapchain._extent.width, swapchain._extent.height, 0.0, 1.0));
	cmd.set_scissor(lvk::info::scissor(0, 0, swapchain._extent.width, swapchain._extent.height));

	// cmd.begin_render_pass(render_pass, swapchain._framebuffers[frame.image_index], swapchain, VK_SUBPASS_CONTENTS_INLINE, clear_value);

	const VkRenderingAttachmentInfo color_attachment_info {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.imageView = swapchain._image_views[frame.image_index],
		.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = {
			.color = {
				{ 0.0f, 0.0f, 0, 0.0f }
			}
		},
	};

	const VkRenderingInfo render_info {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.renderArea = {
			.offset = { 0, 0 },
			.extent = swapchain._extent
		},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_attachment_info,
	};
	

	vkCmdBeginRendering(cmd._command_buffer, &render_info);

	cmd.bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

	cmd.bind_vertex_buffers({ mesh.vertex_buffer }, { 0 });
	cmd.bind_index_buffer(mesh.index_buffer, VK_INDEX_TYPE_UINT32);
	cmd.bind_descriptor_set(VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, { global_descriptor });

	cmd.draw_indexed(mesh.indices.size(), 1, 0, 0, 0);

	vkCmdEndRendering(cmd._command_buffer);

	{
		const VkImageMemoryBarrier image_memory_barrier {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.image = swapchain._images[frame.image_index],
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			}
		};

		vkCmdPipelineBarrier(cmd._command_buffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &image_memory_barrier);
	}

	cmd.end();
}

void lucy::renderer::submit(const lre_frame& frame) {
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

	device.submit(&submit_info, 1, frame.render_fence);
	
	// vkWaitForFences(device._device, 1, &frame.render_fence._fence, false, LVK_TIMEOUT);
	device.wait_for_fences({ frame.render_fence });
	device.reset_fences({ frame.render_fence });
	// vkResetFences(device._device, 1, &frame.render_fence._fence);

	if (device.present(frame.image_index, swapchain, frame.render_semaphore) == VK_ERROR_OUT_OF_DATE_KHR) {
		resize_requested = true;
	}
	
	// printf("jsdabsfj\n");
}

void lucy::renderer::set_projection(const glm::mat4& projection) {
	mvp.projection = projection;
}

void lucy::renderer::set_view(const glm::mat4& view) {
	mvp.view = view;
}

void lucy::renderer::set_model(const glm::mat4& model) {
	mvp.model = model;
}

void lucy::renderer::update(const bool& is_resized) {
	static uint32_t frame_number = 0;

	if (is_resized || resize_requested) {
		device.wait_idle();
		int width, height;
		SDL_GetWindowSize(sdl_window, &width, &height);
		device.swapchain_recreate(swapchain, render_pass, width, height);

		frame_number = 0;
		resize_requested = false;
		return;
	}

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	//some imgui UI to test
	ImGui::ShowDemoWindow();

	//make imgui calculate internal draw structures
	ImGui::Render();


	if (frame_number > 0) {
		submit(frame_array[(frame_number - 1) % FRAMES_IN_FLIGHT]);
	}

	record(frame_array[frame_number % FRAMES_IN_FLIGHT]);

	frame_number++;
}

void lucy::renderer::destroy() {
	dloggln("-----------------------------------------------------------");

	device.swapchain_destroy(swapchain);
	device.wait_idle();

	deletor.flush();
	
	device.destroy();
	instance.destroy();
}
