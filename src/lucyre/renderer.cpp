#include "lucyvk/functions.h"
#include "lucyvk/create_info.h"
#include "lucyvk/shaders.h"

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

void lre::renderer::init_frame_data() {
	command_pool = device.create_graphics_command_pool();

	for (int i = 0; i < FRAMES_IN_FLIGHT; i++) {
		frame_array[i].command_buffer = device.allocate_command_buffer_unique(command_pool);

		frame_array[i].render_fence = device.create_fence();
		frame_array[i].render_semaphore = device.create_semaphore();
		frame_array[i].present_semaphore = device.create_semaphore();
	}
}

void lre::renderer::init_upload_mesh() {
	
}

void lre::renderer::init_swapchain(glm::ivec2 size) {
	swapchain = device.create_swapchain(size.x, size.y, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, {
		.format = VK_FORMAT_B8G8R8A8_UNORM,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
	});
}

void lre::renderer::init_descriptor_pool() {
	uint32_t descriptor_set_max_size = 10;
	VkDescriptorPoolSize descriptor_pool_sizes[] = {
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }
	};

	lvk_descriptor_pool descriptor_pool = device.create_descriptor_pool(descriptor_set_max_size, descriptor_pool_sizes);

	// seperate for each shader type
	descriptor_set_layout = device.create_descriptor_set_layout({
		lvk::descriptor_set_layout_binding(0, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),
		lvk::descriptor_set_layout_binding(1, VK_SHADER_STAGE_VERTEX_BIT, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
	});

	descriptor = device.create_descriptor_set(descriptor_pool, descriptor_set_layout);

	// binding for uniform buffer
	device.update_descriptor_set(descriptor, 1, &mvp_uniform_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0);
	// descriptor.update(0, &compute_image_view, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
}

void lre::renderer::init_render_pass() {
	depth_image = allocator.create_image(VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
	depth_image_view = device.create_image_view(depth_image, VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_VIEW_TYPE_2D);

	render_pass = device.create_default_render_pass(swapchain._surface_format.format);
	framebuffer_array = new lvk_framebuffer[swapchain._image_count];

	for (int i = 0; i < swapchain._image_count; i++) {
		framebuffer_array[i] = device.create_framebuffer(render_pass, swapchain._extent, { swapchain._image_views[i], depth_image_view._image_view });
	}
}

void lre::renderer::init_pipeline() {
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

	graphics_pipeline_layout = device.create_pipeline_layout({
		descriptor_set_layout._descriptor_set_layout
	});
	graphics_pipeline = device.create_graphics_pipeline(graphics_pipeline_layout, &config, &render_pass);
	
	lvk_shader_module compute_shader = device.init_shader_module(VK_SHADER_STAGE_COMPUTE_BIT, "/home/laperex/Programming/C++/LucyVK/build/shaders/gradient.comp.spv");
	
	// compute_pipeline_layout = device.init_pipeline_layout({
	// 	descriptor_set_layout._descriptor_set_layout
	// });
	// compute_pipeline = compute_pipeline_layout.init_compute_pipeline(lvk::info::shader_stage(&compute_shader));
}

void lre::renderer::upload_mesh(Mesh& mesh) {
	mesh.vertex_buffer = allocator.create_vertex_buffer(mesh.vertices.data(), mesh.vertices.size());
	
}

void lre::renderer::initialization(SDL_Window* window) {
	lvk::config::instance instance_config = {
		.name = "Lucy Framework v7",
		.enable_validation_layers = true
	};

	instance = lvk_instance::init(&instance_config, window);
	// physical_device = instance.init_device();
	device = instance.create_device({ VK_KHR_SWAPCHAIN_EXTENSION_NAME });
	
	allocator = device.create_allocator();
	
	mvp_uniform_buffer = allocator.create_uniform_buffer<mvp_matrix>();

	init_frame_data();
	
	sdl_window = window;
	glm::ivec2 size;
	SDL_GetWindowSize(sdl_window, &size.x, &size.y);
	init_swapchain(size);

	// compute_image = allocator.init_image(VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT, { swapchain._extent.width, swapchain._extent.height, 1 }, VK_IMAGE_TYPE_2D);
	// compute_image_view = compute_image.init_image_view(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D);

	// immediate_command_buffer = command_pool.init_immediate_command_buffer();
	
	// immediate_command_buffer.transition_image(compute_image._image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	
	// lvk_init_sampler

	init_descriptor_pool();

	init_render_pass();
	init_pipeline();
	
	
	immediate_command = device.create_immediate_command();
	device.immediate_submit(immediate_command, [=](const VkCommandBuffer cmd){
		
	});
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
		.model = glm::rotate(glm::mat4(1.0f), glm::radians(frame_number * 0.4f), glm::vec3(0, 1, 0)),
		.color = { 0, 1, 0, 1},
	};
	
	mvp.projection[1][1] *= -1;

	// mvp_uniform_buffer.upload(mvp);
	allocator.upload(mvp_uniform_buffer, mvp);

	cmd.reset();
	
	// Allo

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

	// cmd.bind_pipeline(&compute_pipeline);
	
	// cmd.transition_image(&compute_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// vkCmdBindDescriptorSets(cmd._command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline_layout._pipeline_layout, 0, 1, &descriptor._descriptor_set, 0, VK_NULL_HANDLE);
	// vkCmdDispatch(cmd._command_buffer, std::ceil(swapchain._extent.width / 16.0), std::ceil(swapchain._extent.height / 16.0), 1);

	cmd.begin_render_pass(&render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
	
	// cmd.bind_vertex_buffers({ terrain.vertex_buffer._buffer }, { 0 });
	cmd.bind_pipeline(&graphics_pipeline);

	vkCmdBindDescriptorSets(cmd._command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_layout._pipeline_layout, 0, 1, &descriptor._descriptor_set, 0, VK_NULL_HANDLE);

	vkCmdDraw(cmd._command_buffer, 6, 1, 0, 0);

	cmd.end_render_pass();
// 	projection = glm::perspective(glm::radians(fov), float(width) / float(height), c_near, c_far);
// }

	// upload.command_pool = device.create_graphics_command_pool();
	// upload.command_buffer = device.allocate_command_buffer_unique(upload.command_pool);
	// upload.fence = device.create_fence();

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
