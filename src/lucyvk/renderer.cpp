#include "lucyvk/functions.h"
#include "lucyvk/create_info.h"
// #include "lucyvk/shaders.h"

// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

// #define VMA_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include <assimp/Importer.hpp>

#include "lucyio/logger.h"

#include "renderer.h"
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>
#include <imgui.h>


lucy::renderer::renderer()
{
	
}


GPUMeshBuffers lucy::renderer::upload_mesh(const std::span<Vertex>& vertices, const std::span<uint32_t>& indices) const {
	std::size_t vertex_buffer_size = vertices.size_bytes();
	std::size_t index_buffer_size = indices.size_bytes();
	
	GPUMeshBuffers mesh_buffers = {
		.index_buffer = device.create_buffer(
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			vertex_buffer_size
		),
		.vertex_buffer = device.create_buffer(
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			index_buffer_size
		)
	};

	VkBufferDeviceAddressInfo device_address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = mesh_buffers.vertex_buffer._buffer
	};

	mesh_buffers.vertexBufferAddress = vkGetBufferDeviceAddress(device.get_logical_device(), &device_address_info);


	lvk_buffer staging = device.create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, vertex_buffer_size + index_buffer_size);

	void* data = staging._allocation->GetMappedData();

	// copy vertex buffer
	memcpy(data, vertices.data(), vertex_buffer_size);
	// copy index buffer
	memcpy((char*)data + vertex_buffer_size, indices.data(), index_buffer_size);

	device.immediate_submit([&](lvk_command_buffer cmd) {
		cmd.copy_buffer(staging._buffer, mesh_buffers.vertex_buffer, {
			{
				.srcOffset = 0,
				.dstOffset = 0,
				.size = vertex_buffer_size,
			}
		});

		cmd.copy_buffer(staging._buffer, mesh_buffers.index_buffer, {
			{
				.srcOffset = vertex_buffer_size,
				.dstOffset = 0,
				.size = index_buffer_size,
			}
		});
	});

	deletor.destroy(staging);

	return mesh_buffers;
}

// FRAME

lre_frame lucy::renderer::create_frame(lvk_command_pool& command_pool) {
	return {
		.render_fence = deletor.push(device.create_fence()),

		.present_semaphore = deletor.push(device.create_semaphore()),
		.render_semaphore = deletor.push(device.create_semaphore()),

		.command_buffer = deletor.push(device.create_command_buffer(command_pool), command_pool),
	};
}

void lucy::renderer::destroy_frame(lre_frame& frame) {
	if (frame.draw_image_view._image_view != VK_NULL_HANDLE) {
		deletor.destroy(frame.draw_image_view);
	}
	if (frame.draw_image._image != VK_NULL_HANDLE) {
		deletor.destroy(frame.draw_image);
	}
}


void lucy::renderer::init_pipeline() {
	// seperate for each shader type
	lvk_descriptor_set_layout descriptor_set_layout = deletor.push(
		device.create_descriptor_set_layout({
			lvk::info::descriptor_set_layout_binding(0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
			lvk::info::descriptor_set_layout_binding(1, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
			lvk::info::descriptor_set_layout_binding(2, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1),
		})
	);

	device.create_descriptor_set(descriptor_pool, descriptor_set_layout);
	
	lvk_shader_module vertex_shader = device.create_shader_module("./shaders/texture.vert.spv");
	lvk_shader_module fragment_shader = device.create_shader_module("./shaders/texture.frag.spv");

	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	lvk::config::graphics_pipeline config = {
		.shader_stage_array = {
			lvk::info::shader_stage(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader),
			lvk::info::shader_stage(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader),
		},

		.vertex_input_state = lvk::info::vertex_input_state(
			{
				lvk::info::vertex_input_description(0, sizeof(glm::vec3) * 3, VK_VERTEX_INPUT_RATE_VERTEX)
			},
			{
				lvk::info::vertex_input_attribute_description(0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) * 0),
				lvk::info::vertex_input_attribute_description(0, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) * 1),
				lvk::info::vertex_input_attribute_description(0, 2, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) * 2),
			}
		),

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
		
		.dynamic_state_array = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		},

		.rendering_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,

			.pNext = VK_NULL_HANDLE,

			.colorAttachmentCount = static_cast<uint32_t>(draw_property.color_format_array.size()),
    		.pColorAttachmentFormats = draw_property.color_format_array.data(),
			.depthAttachmentFormat = draw_property.depth_format,
			.stencilAttachmentFormat = draw_property.stencil_format
		}
	};

	lvk_pipeline_layout graphics_pipeline_layout = deletor.push(device.create_pipeline_layout({ descriptor_set_layout }));
	lvk_pipeline graphics_pipeline = deletor.push(device.create_graphics_pipeline_dynamic(graphics_pipeline_layout, config));

	deletor.destroy(fragment_shader);
	deletor.destroy(vertex_shader);
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

void lucy::renderer::draw_imgui(lre_frame& frame) {
	const auto& cmd = frame.command_buffer;

	cmd.begin_rendering(swapchain._extent, {
		lvk::info::rendering_attachment_info(swapchain._image_views[frame.swapchain_image_index], VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR)
	});

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	cmd.end_rendering();
}

void lucy::renderer::draw_background(lre_frame& frame) {
	const auto& cmd = frame.command_buffer;
	
	static float framenumber = 0;
	// float flash = std::abs(std::sin((framenumber++) / 120.f));
	float flash = 0;

	cmd.clear_color_image(frame.draw_image, VK_IMAGE_LAYOUT_GENERAL, { { 0.0f, 0.0f, flash, 1.0f } }, VK_IMAGE_ASPECT_COLOR_BIT);
}

void lucy::renderer::draw_main(lre_frame& frame) {
	const auto& cmd = frame.command_buffer;

	cmd.set_viewport(lvk::info::viewport(0, 0, swapchain._extent.width, swapchain._extent.height, 0.0, 1.0));
	cmd.set_scissor(lvk::info::scissor(0, 0, swapchain._extent.width, swapchain._extent.height));

	cmd.begin_rendering(
		{
			.width = frame.draw_image._extent.width,
			.height = frame.draw_image._extent.height
		},
		{
			lvk::info::rendering_attachment_info(frame.draw_image_view, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR)
		}
	);

	cmd.end_rendering();
}


void lucy::renderer::init(SDL_Window* window) {
	instance = lvk_instance::initialize("Lucy Framework v17", window, true);
	device = instance.create_device({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME });
	deletor = device.create_deletor();

	sdl_window = window;

	glm::ivec2 size;
	SDL_GetWindowSize(window, &size.x, &size.y);

	swapchain = device.create_swapchain(size.x, size.y, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	});


	lvk_command_pool command_pool = deletor.push(device.create_graphics_command_pool());

	for (auto& frame: frame_array) {
		frame = create_frame(command_pool);
	}
	
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

	draw_property = {
		.color_format_array = { VK_FORMAT_R16G16B16A16_SFLOAT },
		.depth_format = VK_FORMAT_D32_SFLOAT
	};

	init_pipeline();

	init_imgui(sdl_window);

	// lvk_image load_image = deletor.push(device.load_image_from_file("/home/laperex/Programming/C++/LucyVK/assets/buff einstein.jpg"));
	// lvk_image_view load_image_view = deletor.push(device.create_image_view(load_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT));
	// lvk_sampler sampler = deletor.push(device.create_sampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT));

	// device.update_descriptor_set(global_descriptor, 2, &load_image_view, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);

	set_model(glm::mat4(1.0f));
}

void lucy::renderer::record(lre_frame& frame) {
	const auto& cmd = frame.command_buffer;

	if (device.swapchain_acquire_next_image(swapchain, &frame.swapchain_image_index, frame.present_semaphore, VK_NULL_HANDLE) == VK_ERROR_OUT_OF_DATE_KHR) {
		resize_requested = true;
		return;
	}

	static lvk_buffer mvp_uniform_buffer = {
		._buffer = VK_NULL_HANDLE
	};

	if (mvp_uniform_buffer._buffer == VK_NULL_HANDLE) {
		mvp_uniform_buffer = deletor.push(device.create_uniform_buffer<decltype(mvp)>());
	}

	device.upload(mvp_uniform_buffer, mvp);


	//* Draw Image Recreation
	if (frame.draw_image._extent.width != swapchain._extent.width || frame.draw_image._extent.height != swapchain._extent.height) {
		if (frame.draw_image._image != VK_NULL_HANDLE) {
			deletor.destroy(frame.draw_image_view);
			deletor.destroy(frame.draw_image);
		}

		frame.draw_image = device.create_image(
			draw_property.color_format_array[0],

			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			
			VMA_MEMORY_USAGE_GPU_ONLY,

			VkExtent3D {
				.width = swapchain._extent.width,
				.height = swapchain._extent.height,
				.depth = 1
			},

			VK_IMAGE_TYPE_2D
		);

		frame.draw_image_view = device.create_image_view(frame.draw_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT);
	}


	//* Command Buffer Recording
	cmd.reset();
	cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	//* draw background
	cmd.transition_image2(frame.draw_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	draw_background(frame);

	//* Main Rendering onto draw_image
	cmd.transition_image2(frame.draw_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	draw_main(frame);

	//! do not edit below
	cmd.transition_image2(frame.draw_image._image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	cmd.transition_image2(swapchain._images[frame.swapchain_image_index], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	cmd.blit_image_to_image2(frame.draw_image, swapchain._images[frame.swapchain_image_index], { frame.draw_image._extent.width, frame.draw_image._extent.height }, swapchain._extent);

	cmd.transition_image2(swapchain._images[frame.swapchain_image_index], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	draw_imgui(frame);

	cmd.transition_image2(swapchain._images[frame.swapchain_image_index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

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

	device.wait_for_fences({ frame.render_fence });
	device.reset_fences({ frame.render_fence });

	if (device.present(frame.swapchain_image_index, swapchain, frame.render_semaphore) == VK_ERROR_OUT_OF_DATE_KHR) {
		resize_requested = true;
	}
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
		device.swapchain_recreate(swapchain, width, height);

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

	for (auto& frame: frame_array) {
		destroy_frame(frame);
	}

	device.swapchain_destroy(swapchain);
	device.wait_idle();

	deletor.flush();
	
	device.destroy();
	instance.destroy();
}
