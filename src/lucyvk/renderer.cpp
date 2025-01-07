#define VMA_IMPLEMENTATION

#include "stb_image.h"
#include <iostream>
// #include <vk_loader.h>

// #include "vk_engine.h"
// #include "vk_initializers.h"
// #include "vk_types.h"
#include <glm/gtx/quaternion.hpp>

#include <fastgltf/glm_element_traits.hpp>
// #include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include "lucyvk/functions.h"
#include <fastgltf/tools.hpp>
#include <fmt/core.h>
#include "lucyvk/create_info.h"
#include <fastgltf/core.hpp>

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
			
			index_buffer_size
		),
		.vertex_buffer = device.create_buffer(
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			vertex_buffer_size
		)
	};

	VkBufferDeviceAddressInfo device_address_info = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = mesh_buffers.vertex_buffer._buffer
	};

	mesh_buffers.vertex_buffer_address = vkGetBufferDeviceAddress(device.get_logical_device(), &device_address_info);


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

		.descriptor_allocator = { device },
	};
}

void lucy::renderer::destroy_frame_images(lre_frame& frame) {
	if (frame.draw_image_view._image_view != VK_NULL_HANDLE) {
		deletor.destroy(frame.draw_image_view);
	}
	if (frame.draw_image._image != VK_NULL_HANDLE) {
		deletor.destroy(frame.draw_image);
	}
	if (frame.depth_image_view._image_view != VK_NULL_HANDLE) {
		deletor.destroy(frame.depth_image_view);
	}
	if (frame.depth_image._image != VK_NULL_HANDLE) {
		deletor.destroy(frame.depth_image);
	}
}


void lucy::renderer::init_pipeline() {
	
	lvk_shader_module vertex_shader = device.create_shader_module("./shaders/colored_triangle_mesh.vert.spv");
	lvk_shader_module fragment_shader = device.create_shader_module("./shaders/colored_triangle.frag.spv");

	VkPipelineColorBlendAttachmentState color_blend_attachment_state = {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
	};

	lvk::config::graphics_pipeline config = {
		.shader_stage_array = {
			lvk::info::shader_stage(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader),
			lvk::info::shader_stage(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader),
		},

		.vertex_input_state = lvk::info::vertex_input_state(),

		.input_assembly_state = lvk::info::input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST),
		
		.rasterization_state = lvk::info::rasterization_state(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE),

		.multisample_state = lvk::info::multisample_state(),

		.depth_stencil_state = lvk::info::depth_stencil_state(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL),

		.color_blend_state = lvk::info::color_blend_state(),
		
		.color_blend_attachments = {
			lvk::info::color_blend_attachment_additive()
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

	// seperate for each shader type
	// lvk_descriptor_set_layout descriptor_set_layout = deletor.push(
	// 	device.create_descriptor_set_layout({
	// 		lvk::info::descriptor_set_layout_binding(0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
	// 		lvk::info::descriptor_set_layout_binding(1, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
	// 		lvk::info::descriptor_set_layout_binding(2, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1),
	// 	})
	// );

	// device.create_descriptor_set(descriptor_pool, descriptor_set_layout);

	mesh_pipeline_layout = deletor.push(device.create_pipeline_layout(
		{
			VkPushConstantRange {
				.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
				.offset = 0,
				.size = sizeof(GPUDrawPushConstants)
			}
		}
	));

	mesh_pipeline = deletor.push(device.create_graphics_pipeline(mesh_pipeline_layout, config));

	deletor.destroy(fragment_shader);
	deletor.destroy(vertex_shader);
}

GPUMeshBuffers lucy::renderer::init_sample_rectangle() {
	std::array<Vertex,4> rect_vertices;

	rect_vertices[0].position = {  0.5, -0.5, 0 };
	rect_vertices[1].position = {  0.5,  0.5, 0 };
	rect_vertices[2].position = { -0.5, -0.5, 0 };
	rect_vertices[3].position = { -0.5,  0.5, 0 };

	rect_vertices[0].color = {   0,   0,   0, 1 };
	rect_vertices[1].color = { 0.5, 0.5, 0.5, 1 };
	rect_vertices[2].color = {   1,   0,   0, 1 };
	rect_vertices[3].color = {   0,   1,   0, 1 };


	std::array<uint32_t,6> rect_indices;

	rect_indices[0] = 0;
	rect_indices[1] = 1;
	rect_indices[2] = 2;

	rect_indices[3] = 2;
	rect_indices[4] = 1;
	rect_indices[5] = 3;


	GPUMeshBuffers rect_buffer = upload_mesh(rect_vertices, rect_indices);

	return rect_buffer;
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
	float flash = std::abs(std::sin((framenumber++) / 120.f));
	// flash = 0;

	cmd.clear_color_image(frame.draw_image, VK_IMAGE_LAYOUT_GENERAL, { { 0.0f, 0.0f, flash, 1.0f } }, VK_IMAGE_ASPECT_COLOR_BIT);
}

void lucy::renderer::draw_geometry(lre_frame& frame) {
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
		},
		lvk::info::rendering_attachment_info(frame.depth_image_view, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR, VkClearValue {
			.depthStencil = {
				.depth = 1.0f
			}
		})
	);

	cmd.bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, mesh_pipeline);

	GPUDrawPushConstants push_constants;
	glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5));
	// camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)frame.draw_image._extent.width / (float)frame.draw_image._extent.height, 10000.f, 0.1f);

	// invert the Y direction on projection matrix so that we are more similar
	// to opengl and gltf axis
	projection[1][1] *= -1;

	push_constants.world_matrix = mvp.projection * mvp.view * mvp.model;
	// push_constants.world_matrix = glm::mat4{ 1.f };
	// push_constants.vertexBuffer = rectangle.vertexBufferAddress;
	push_constants.vertex_buffer = test_meshes[2]->mesh_buffers.vertex_buffer_address;

	cmd.push_constants(mesh_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, push_constants);
	cmd.bind_index_buffer(test_meshes[2]->mesh_buffers.index_buffer, 0, VK_INDEX_TYPE_UINT32);

	cmd.draw_indexed(test_meshes[2]->surfaces[0].count, 1, test_meshes[2]->surfaces[0].start_index, 0, 0);

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
		
		std::vector<lvk_descriptor_allocator_growable::PoolSizeRatio> frame_sizes = { 
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
		};

		frame.descriptor_allocator.init(1000, frame_sizes);
	
		deletor.push_fn([&]() {
			frame.descriptor_allocator.destroy_pools();
		});
	}

	draw_property = {
		.color_format_array = { VK_FORMAT_R16G16B16A16_SFLOAT },
		.depth_format = VK_FORMAT_D32_SFLOAT
	};

	init_pipeline();
	rectangle = init_sample_rectangle();

	deletor.push(rectangle.vertex_buffer);
	deletor.push(rectangle.index_buffer);

	init_imgui(sdl_window);

	// lvk_image load_image = deletor.push(device.load_image_from_file("/home/laperex/Programming/C++/LucyVK/assets/buff einstein.jpg"));
	// lvk_image_view load_image_view = deletor.push(device.create_image_view(load_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_COLOR_BIT));
	// lvk_sampler sampler = deletor.push(device.create_sampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT));

	// device.update_descriptor_set(global_descriptor, 2, &load_image_view, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
	
	{
		//3 default textures, white, grey, black. 1 pixel each
		uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
		// white_image = create_image((void*)&white, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
		white_image = device.create_image(
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY, VkExtent3D{1, 1, 1}, VK_IMAGE_TYPE_2D
		);

		uint32_t grey = glm::packUnorm4x8(glm::vec4(0.66f, 0.66f, 0.66f, 1));
		// grey_image = create_image((void*)&grey, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
		grey_image = device.create_image(
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY, VkExtent3D{1, 1, 1}, VK_IMAGE_TYPE_2D
		);

		uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
		black_image = device.create_image(
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY, VkExtent3D{1, 1, 1}, VK_IMAGE_TYPE_2D
		);

		//checkerboard image
		uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
		std::array<uint32_t, 16 * 16> pixels; //for 16x16 checkerboard texture
		for (int x = 0; x < 16; x++) {
			for (int y = 0; y < 16; y++) {
				pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta: black;
			}
		}

		error_checkerboard_image = device.create_image(
			VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY, VkExtent3D{16, 16, 1}, VK_IMAGE_TYPE_2D
		);

		default_sampler_linear = device.create_sampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
		default_sampler_nearest = device.create_sampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	}
	
	gpu_descriptor_set_layout = deletor.push(device.create_descriptor_set_layout({
		lvk::info::descriptor_set_layout_binding(0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
	}));

	test_meshes = load_GLTF("/home/laperex/Programming/C++/LucyVK/assets/basicmesh.glb").value();

	for (auto& mesh: test_meshes) {
		deletor.push(mesh->mesh_buffers.index_buffer);
		deletor.push(mesh->mesh_buffers.vertex_buffer);
	}

	gpu_scene_data_buffer = deletor.push(device.create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, sizeof(GPUSceneData)));

	set_model(glm::mat4(1.0f));
}

void lucy::renderer::record(lre_frame& frame) {
	const auto& cmd = frame.command_buffer;
	
	//write the buffer
	GPUSceneData* scene_uniform_data = (GPUSceneData*)gpu_scene_data_buffer._allocation->GetMappedData();
	*scene_uniform_data = sceneData;

	//create a descriptor set that binds that buffer and update it
	VkDescriptorSet global_descriptor = frame.descriptor_allocator.allocate(gpu_descriptor_set_layout);

	lvk_descriptor_writer writer = { device };
	writer.write_buffer(0, gpu_scene_data_buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.update_set(global_descriptor);

	if (device.swapchain_acquire_next_image(swapchain, &frame.swapchain_image_index, frame.present_semaphore, VK_NULL_HANDLE) == VK_ERROR_OUT_OF_DATE_KHR) {
		resize_requested = true;
		return;
	}

	// static lvk_buffer mvp_uniform_buffer = {
	// 	._buffer = VK_NULL_HANDLE
	// };

	// if (mvp_uniform_buffer._buffer == VK_NULL_HANDLE) {
	// 	mvp_uniform_buffer = deletor.push(device.create_uniform_buffer<decltype(mvp)>());
	// }

	// device.upload(mvp_uniform_buffer, mvp);


	//* Draw Image Recreation
	if (frame.draw_image._extent.width != swapchain._extent.width || frame.draw_image._extent.height != swapchain._extent.height) {
		if (frame.draw_image._image != VK_NULL_HANDLE) {
			destroy_frame_images(frame);
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

		frame.depth_image = device.create_image(
			draw_property.depth_format,

			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,

			VMA_MEMORY_USAGE_GPU_ONLY,
			
			VkExtent3D {
				.width = swapchain._extent.width,
				.height = swapchain._extent.height,
				.depth = 1
			},

			VK_IMAGE_TYPE_2D
		);
		
		frame.depth_image_view = device.create_image_view(frame.depth_image, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
	}


	//* Command Buffer Recording
	cmd.reset();
	cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	//* draw background
	cmd.transition_image2(frame.draw_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	draw_background(frame);

	// //* Main Rendering onto draw_image
	cmd.transition_image2(frame.draw_image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	cmd.transition_image2(frame.depth_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	draw_geometry(frame);

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

std::optional<std::vector<std::shared_ptr<MeshAsset>>> lucy::renderer::load_GLTF(std::filesystem::path filePath) {
	std::cout << "Loading GLTF: " << filePath << std::endl;

    fastgltf::GltfDataBuffer data;
    data.loadFromFile(filePath);

    constexpr auto gltfOptions = fastgltf::Options::LoadGLBBuffers | fastgltf::Options::LoadExternalBuffers;

    fastgltf::Asset gltf;
    fastgltf::Parser parser {};

    auto load = parser.loadGltfBinary(&data, filePath.parent_path(), gltfOptions);
    if (load) {
        gltf = std::move(load.get());
    } else {
        fmt::print("Failed to load glTF: {} \n", fastgltf::to_underlying(load.error()));
        return {};
    }

	std::vector<std::shared_ptr<MeshAsset>> meshes;

    // use the same vectors for all meshes so that the memory doesnt reallocate as
    // often
    std::vector<uint32_t> indices;
    std::vector<Vertex> vertices;
    for (fastgltf::Mesh& mesh : gltf.meshes) {
        MeshAsset newmesh;

        newmesh.name = mesh.name;

        // clear the mesh arrays each mesh, we dont want to merge them by error
        indices.clear();
        vertices.clear();

        for (auto&& p : mesh.primitives) {
            GeoSurface newSurface;
            newSurface.start_index = (uint32_t)indices.size();
            newSurface.count = (uint32_t)gltf.accessors[p.indicesAccessor.value()].count;

            size_t initial_vtx = vertices.size();

            // load indexes
            {
                fastgltf::Accessor& indexaccessor = gltf.accessors[p.indicesAccessor.value()];
                indices.reserve(indices.size() + indexaccessor.count);

                fastgltf::iterateAccessor<std::uint32_t>(gltf, indexaccessor,
                    [&](std::uint32_t idx) {
                        indices.push_back(idx + initial_vtx);
                    });
            }

            // load vertex positions
            {
                fastgltf::Accessor& posAccessor = gltf.accessors[p.findAttribute("POSITION")->second];
                vertices.resize(vertices.size() + posAccessor.count);

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, posAccessor,
                    [&](glm::vec3 v, size_t index) {
                        Vertex newvtx;
                        newvtx.position = v;
                        newvtx.normal = { 1, 0, 0 };
                        newvtx.color = glm::vec4 { 1.f };
                        newvtx.uv_x = 0;
                        newvtx.uv_y = 0;
                        vertices[initial_vtx + index] = newvtx;
                    });
            }

            // load vertex normals
            auto normals = p.findAttribute("NORMAL");
            if (normals != p.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec3>(gltf, gltf.accessors[(*normals).second],
                    [&](glm::vec3 v, size_t index) {
                        vertices[initial_vtx + index].normal = v;
                    });
            }

            // load UVs
            auto uv = p.findAttribute("TEXCOORD_0");
            if (uv != p.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec2>(gltf, gltf.accessors[(*uv).second],
                    [&](glm::vec2 v, size_t index) {
                        vertices[initial_vtx + index].uv_x = v.x;
                        vertices[initial_vtx + index].uv_y = v.y;
                    });
            }

            // load vertex colors
            auto colors = p.findAttribute("COLOR_0");
            if (colors != p.attributes.end()) {

                fastgltf::iterateAccessorWithIndex<glm::vec4>(gltf, gltf.accessors[(*colors).second],
                    [&](glm::vec4 v, size_t index) {
                        vertices[initial_vtx + index].color = v;
                    });
            }
            newmesh.surfaces.push_back(newSurface);
        }

        // display the vertex normals
        constexpr bool OverrideColors = true;
        if (OverrideColors) {
            for (Vertex& vtx : vertices) {
                vtx.color = glm::vec4(vtx.normal, 1.f);
            }
        }
        newmesh.mesh_buffers = upload_mesh(vertices, indices);

        meshes.emplace_back(std::make_shared<MeshAsset>(std::move(newmesh)));
    }

    return meshes;
}

void lucy::renderer::destroy() {
	dloggln("-----------------------------------------------------------");

	for (auto& frame: frame_array) {
		destroy_frame_images(frame);
	}

	device.swapchain_destroy(swapchain);
	device.wait_idle();

	deletor.flush();
	
	device.destroy();
	instance.destroy();
}
