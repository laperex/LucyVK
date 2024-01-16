

#include "lucyvk/vk_config.h"
#include "lucyvk/vk_function.h"
#include "lucyvk/vk_info.h"
#include "lucyvk/vk_instance.h"
#include "lucyvk/vk_render_pass.h"
#include "sdl_window.h"
#include "vk_renderer.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "stb_image.h"

#include "Camera.h"
// #include "Mesh.h"

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
// #include <Window.h>
#include <glm/glm.hpp>

#include <glm/gtx/transform.hpp>

#include <util/logger.h>

#include "lucyvk/lucyvk.h"

#include <glm/gtc/noise.hpp>

int main(int count, char** args) {
	lucy::sdl_window window;
	
	window.initialize("Lucy", { 0, 0 }, { 1920, 1080 });
	
	lucy::vk_renderer renderer;
	
	renderer.initialization(&window);
	
	double dt = 0;
	uint32_t frame_number = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		// TODO: Shader Module creation/deletion/query management
		// TODO: vk_shaders.h
		// TODO: maybe restructure init_ based creation
		// TODO: Remove deletion_queue and find a better approach
		// TODO: Better approach for PhysicalDevice selection and Initialization
		// TODO: Add delta time Macros
		// TODO: multi_init feature for initialization of multiple vulkan types

		renderer.record(frame_number);

		if (frame_number > 0) {
			renderer.submit(frame_number - 1);
		}

		frame_number += 1;
		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
	
	// device.wait_idle();

	// window.Destroy();
}