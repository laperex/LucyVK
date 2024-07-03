#include "lucyvk/functions.h"
#include "lucyvk/create_info.h"
// #include "lucyvk/shaders.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "lucyio/logger.h"

#include "lucyre/renderer.h"


struct mvp_matrix {
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
	glm::vec4 color;
};

lre::renderer::renderer()
{
	
}


void lre::renderer::upload_mesh(Mesh& mesh) {
	mesh.vertex_buffer = allocator.create_vertex_buffer(mesh.vertices.data(), mesh.vertices.size());
	
}

lvk_image lre::renderer::load_image_from_file(const char* filename) {
	int width, height, channels;

	stbi_uc* pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels) {
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

	lvk_buffer staging_buffer = this->allocator.create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, pixels, image_size);

	lvk_image image = this->allocator.create_image(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, image_extent, VK_IMAGE_TYPE_2D);

	this->device.immediate_submit(immediate_command, [&](VkCommandBuffer cmd) {
		lvk_command_buffer command_buffer = static_cast<lvk_command_buffer>(cmd);

		VkImageMemoryBarrier image_barrier = lvk::info::image_memory_barrier(image, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, lvk::info::image_subresource_range(VK_IMAGE_ASPECT_COLOR_BIT));

		command_buffer.pipeline_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, image_barrier);
		command_buffer.copy_buffer_to_image(staging_buffer, image, image._extent);

		image_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		image_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		image_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		image_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		command_buffer.pipeline_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, image_barrier);
	});

	stbi_image_free(pixels);

	return image;
}

void lre::renderer::texture_pipeline_init() {
	lvk_shader_module vertex_shader = device.create_shader_module("./shaders/texture.vert.spv");
	lvk_shader_module fragment_shader = device.create_shader_module("./shaders/texture.frag.spv");


	VkViewport viewport[] = {
		{
			.x = 0.0f,
			.y = 0.0f,

			.width = static_cast<float>(swapchain._extent.width),
			.height = static_cast<float>(swapchain._extent.height),

			.minDepth = 0.0f,
			.maxDepth = 1.0f
		}
	};

	VkRect2D scissor[] = {
		{
			.offset = { 0, 0 },
			.extent = { static_cast<uint32_t>(swapchain._extent.width), static_cast<uint32_t>(swapchain._extent.height) }
		}
	};


	VertexInputDescription vertex_description = Vertex::get_vertex_input_description();


	lvk::config::graphics_pipeline config = {
		.shader_stage_array = {
			lvk::info::shader_stage(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader),
			lvk::info::shader_stage(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader),
		},

		.vertex_input_state = lvk::info::vertex_input_state(vertex_description.bindings, vertex_description.attributes),

		.input_assembly_state = lvk::info::input_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE),

		.viewport_state = lvk::info::viewport_state(viewport, scissor),
		
		.rasterization_state = lvk::info::rasterization_state(VK_POLYGON_MODE_FILL),

		.multisample_state = lvk::info::multisample_state(),

		.depth_stencil_state = lvk::info::depth_stencil_state(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL),

		.color_blend_state = lvk::info::color_blend_state({
			{
				.blendEnable = VK_FALSE,
				.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
			}
		}),
	};

	graphics_pipeline_layout = device.create_pipeline_layout({ descriptor_set_layout });
	graphics_pipeline = device.create_graphics_pipeline(graphics_pipeline_layout, config, render_pass);
}

void lre::renderer::descriptor_set_init() {
	uint32_t descriptor_set_max_size = 10;

	VkDescriptorPoolSize descriptor_pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
	};

	descriptor_pool = device.create_descriptor_pool(descriptor_set_max_size, descriptor_pool_sizes);

	// seperate for each shader type
	descriptor_set_layout = device.create_descriptor_set_layout({
		lvk::descriptor_set_layout_binding(0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
		lvk::descriptor_set_layout_binding(1, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
		lvk::descriptor_set_layout_binding(2, VK_SHADER_STAGE_FRAGMENT_BIT, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1),
	});

	descriptor_ubo = device.create_descriptor_set(descriptor_pool, descriptor_set_layout);
}

void lre::renderer::init(SDL_Window* window) {
	lvk::config::instance instance_config = {
		.name = "Lucy Framework v7",
		.enable_validation_layers = true
	};

	instance = lvk_instance::init(&instance_config, window);
	device = instance.create_device({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
	allocator = device.create_allocator();

	mvp_uniform_buffer = allocator.create_uniform_buffer<mvp_matrix>();

	command_pool = device.create_graphics_command_pool();

	for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
		frame_array[i].command_buffer = device.allocate_command_buffer_unique(command_pool);

		frame_array[i].render_fence = device.create_fence();
		frame_array[i].render_semaphore = device.create_semaphore();
		frame_array[i].present_semaphore = device.create_semaphore();
	}
	
	
	mesh.vertices = std::vector<Vertex> {
		{ {  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 0.0f } },
		{ { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f } },
		{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
	};
	
	mesh.indices = { 0,1,2, 2,3,0 };
	
	mesh.index_buffer = allocator.create_index_buffer(mesh.indices);
	
	mesh.vertex_buffer = allocator.create_vertex_buffer(mesh.vertices);


	glm::ivec2 size;
	SDL_GetWindowSize(window, &size.x, &size.y);

	swapchain = device.create_swapchain(size.x, size.y, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	});

	descriptor_set_init();
	
	

	// binding for uniform buffer
	device.update_descriptor_set(descriptor_ubo, 1, &mvp_uniform_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);


	depth_image = allocator.create_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
	depth_image_view = device.create_image_view(depth_image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D);

	render_pass = device.create_default_render_pass(swapchain._surface_format.format);
	framebuffer_array.reserve(swapchain._image_count);

	for (int i = 0; i < swapchain._image_count; i++) {
		framebuffer_array[i] = device.create_framebuffer(render_pass, swapchain._extent, { swapchain._image_views[i], depth_image_view._image_view });
	}

	texture_pipeline_init();

	immediate_command = device.create_immediate_command();

	load_image = load_image_from_file("/home/laperex/Programming/C++/LucyVK/assets/buff einstein.jpg");

	load_image_view = device.create_image_view(load_image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);
	sampler = device.create_sampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	device.update_descriptor_set(descriptor_ubo, 2, &load_image_view, sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0);
}

void lre::renderer::record(uint32_t frame_number) {
	auto& frame = frame_array[frame_number % FRAMES_IN_FLIGHT];
	auto& cmd = frame.command_buffer;

	// swapchain.acquire_next_image(&frame.image_index, frame.present_semaphore._semaphore, VK_NULL_HANDLE);
	device.swapchain_acquire_next_image(swapchain, &frame.image_index, frame.present_semaphore._semaphore, VK_NULL_HANDLE);

	// draw.extent = *(const VkExtent2D*)&draw.image._extent;

	glm::vec3 cam_pos = { 0.f, 0.f, -10 };

	mvp_matrix mvp = {
		.projection = glm::perspective(glm::radians(70.f), float(swapchain._extent.width) / float(swapchain._extent.height), 0.1f, 200.0f),
		.view = glm::translate(glm::mat4(1.f), cam_pos),
		.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0, 1, 0)),
		.color = { 0, 1, 0, 1},
	};

	mvp.projection[1][1] *= -1;

	allocator.upload(mvp_uniform_buffer, mvp);


	cmd.reset();

	cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	cmd.begin_render_pass(render_pass, framebuffer_array[frame.image_index], swapchain, VK_SUBPASS_CONTENTS_INLINE, clear_value);
	
	cmd.bind_pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

	cmd.bind_vertex_buffers({ mesh.vertex_buffer._buffer }, { 0 }, 0);
	cmd.bind_index_buffer(mesh.index_buffer, VK_INDEX_TYPE_UINT32);
	cmd.bind_descriptor_set(VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout, { descriptor_ubo });


	vkCmdDrawIndexed(cmd, mesh.indices.size(), 1, 0, 0, 0);


	cmd.end_render_pass();

	cmd.end();
}

void lre::renderer::submit(uint32_t frame_number) {
	auto& frame = frame_array[frame_number % FRAMES_IN_FLIGHT];

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
	
	vkWaitForFences(device._device, 1, &frame.render_fence._fence, false, LVK_TIMEOUT);
	vkResetFences(device._device, 1, &frame.render_fence._fence);

	VkPresentInfoKHR present_info = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &frame.render_semaphore._semaphore,
		
		.swapchainCount = 1,
		.pSwapchains = &swapchain._swapchain,
		
		.pImageIndices = &frame.image_index,
	};

	device.present(&present_info);
}

void lre::renderer::update() {
	static uint32_t frame_number = 0;

	record(frame_number);

	if (frame_number > 0) {
		// dloggln(frame_number);
		submit(frame_number - 1);
	}
	
	frame_number++;
	
	// if (frame_number == 2)
	// 	exit(0);
}

void lre::renderer::destroy() {
	device.wait_idle();
	
	// deletion_queue.flush(device._device);
	
	// vkDestroySemaphore(device._device, sema)

	device.destroy();
	instance.destroy();
}
