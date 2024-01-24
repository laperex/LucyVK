#include "lvk/functions.h"
#include "lvk/create_info.h"
#include "lvk/shaders.h"

#include "util/logger.h"

#include "engine/renderer.h"

struct mvp_matrix {
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 model;
	glm::vec4 color;
};

void lucy::renderer::init_frame_data() {
	for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
		frame_array[i].command_pool = device.init_command_pool();
		frame_array[i].command_buffer = frame_array[i].command_pool.init_command_buffer();

		frame_array[i].render_fence = device.init_fence();

		frame_array[i].render_semaphore = device.init_semaphore();
		frame_array[i].present_semaphore = device.init_semaphore();
	}
}

void lucy::renderer::init_swapchain(glm::ivec2 size) {
	swapchain = device.init_swapchain(size.x, size.y, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	});
}

void lucy::renderer::init_descriptor_pool() {
	uint32_t descriptor_set_max_size = 10;
	VkDescriptorPoolSize descriptor_pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }
	};

	lvk_descriptor_pool descriptor_pool = device.init_descriptor_pool(descriptor_set_max_size, descriptor_pool_sizes);

	// seperate for each shader type
	descriptor_set_layout = device.init_descriptor_set_layout({
		lvk::descriptor_set_layout_binding(0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
		lvk::descriptor_set_layout_binding(1, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
	});

	descriptor = descriptor_pool.init_descriptor_set(&descriptor_set_layout);

	// binding for uniform buffer
	descriptor.update(1, &mvp_uniform_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	// compute_descriptor.update(0, &compute_image_view, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
}

void lucy::renderer::init_render_pass() {
	depth_image = allocator.init_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
	depth_image_view = depth_image.init_image_view(VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D);

	render_pass = device.init_default_render_pass(swapchain._surface_format.format);
	framebuffer_array = new lvk_framebuffer[swapchain._image_count];

	for (int i = 0; i < swapchain._image_count; i++) {
		framebuffer_array[i] = render_pass.init_framebuffer(swapchain._extent, { swapchain._image_views[i], depth_image_view._image_view });
	}
}

void lucy::renderer::init_pipeline() {
	lvk_shader_module vertex_shader = device.init_shader_module(VK_SHADER_STAGE_VERTEX_BIT, "./shaders/colored_triangle.vert.spv");
	lvk_shader_module fragment_shader = device.init_shader_module(VK_SHADER_STAGE_FRAGMENT_BIT, "./shaders/colored_triangle.frag.spv");
	
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
	
	VkVertexInputBindingDescription bindings[] = {
		{
			.binding = 0,
			.stride = sizeof(glm::vec3),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
		}
	};
	
	VkVertexInputAttributeDescription attributes[] = {
		{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = 0,
		}
	};

	lvk::config::graphics_pipeline config = {
		.shader_stage_array = {
			lvk::info::shader_stage(&vertex_shader),
			lvk::info::shader_stage(&fragment_shader),
		},

		.vertex_input_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

			// .vertexBindingDescriptionCount = std::size(bindings),
			// .pVertexBindingDescriptions = bindings,

			// .vertexAttributeDescriptionCount = std::size(attributes),
			// .pVertexAttributeDescriptions = attributes,
		},

		.input_assembly_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,

			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		},

		.viewport_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			
			.viewportCount = 1,
			.pViewports = viewport,
			
			.scissorCount = 1,
			.pScissors = scissor
		},
		
		.rasterization_state = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			
			.lineWidth = 1.0,
		},

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

	graphics_pipeline_layout = device.init_pipeline_layout({
		descriptor_set_layout._descriptor_set_layout
	});
	graphics_pipeline = graphics_pipeline_layout.init_graphics_pipeline(&config, &render_pass);
	
	lvk_shader_module compute_shader = device.init_shader_module(VK_SHADER_STAGE_COMPUTE_BIT, "./shaders/gradient.comp.spv");
	
	compute_pipeline_layout = device.init_pipeline_layout({
		descriptor_set_layout._descriptor_set_layout
	});
	compute_pipeline = compute_pipeline_layout.init_compute_pipeline(lvk::info::shader_stage(&compute_shader));
}

void lucy::renderer::initialization(lucy::window* window) {
	lvk::config::instance instance_config = {
		.name = "Lucy Framework v7",
		.enable_validation_layers = true
	};

	instance = lvk_init_instance(&instance_config, window->window);
	physical_device = instance.init_physical_device();
	device = physical_device.init_device({ VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME });
	
	allocator = device.init_allocator();
	
	mvp_uniform_buffer = allocator.init_uniform_buffer<mvp_matrix>();

	init_frame_data();
	
	init_swapchain(window->size());

	compute_image = allocator.init_image(VK_FORMAT_R16G16B16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
	compute_image_view = compute_image.init_image_view(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);

	init_descriptor_pool();

	init_render_pass();
	init_pipeline();
}

void lucy::renderer::record(uint32_t frame_number) {
	auto& frame = frame_array[frame_number % FRAMES_IN_FLIGHT];
	auto& cmd = frame.command_buffer;

	swapchain.acquire_next_image(&frame.image_index, frame.present_semaphore._semaphore, VK_NULL_HANDLE);

	// draw.extent = *(const VkExtent2D*)&draw.image._extent;

	glm::vec3 camPos = { 0.f,0.f, -10 };

	mvp_matrix mvp = {
		.projection = glm::perspective(glm::radians(70.f), float(swapchain._extent.width) / float(swapchain._extent.height), 0.1f, 200.0f),
		.view = glm::translate(glm::mat4(1.f), camPos),
		.model = glm::rotate(glm::mat4(1.0f), glm::radians(frame_number * 0.4f), glm::vec3(0, 1, 0)),
		.color = { 0, 1, 0, 1},
	};
	
	mvp.projection[1][1] *= -1;

	mvp_uniform_buffer.upload(mvp);

	cmd.reset();

	cmd.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VkRenderPassBeginInfo render_pass_begin_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = render_pass._render_pass,
		.framebuffer = framebuffer_array[frame.image_index]._framebuffer,

		.renderArea = {
			.offset = { 0, 0 },
			.extent = swapchain._extent,
		},

		.clearValueCount = static_cast<uint32_t>(std::size(clear_value)),
		.pClearValues = clear_value
	};
	
	cmd.begin_render_pass(&render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	
	// cmd.bind_vertex_buffers({ terrain.vertex_buffer._buffer }, { 0 });
	cmd.bind_pipeline(&graphics_pipeline);

	vkCmdBindDescriptorSets(cmd._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout._pipeline_layout, 0, 1, &descriptor._descriptor_set, 0, VK_NULL_HANDLE);

	vkCmdDraw(cmd._command_buffer, 6, 1, 0, 0);

	cmd.end_render_pass();
// 	projection = glm::perspective(glm::radians(fov), float(width) / float(height), c_near, c_far);
// }


	cmd.end();
}

void lucy::renderer::submit(uint32_t frame_number) {
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

void lucy::renderer::update() {
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

void lucy::renderer::destroy() {
	device.wait_idle();

	device.destroy();
	instance.destroy();
}
