// #include "lucy/engine.h"
#include "lucyio/logger.h"
#include <lucy/engine.h>
#include <stdio.h>
#include <vulkan/vulkan_core.h>


// // TODO: dynamic viewport
// TODO: renderer restructuring
	// TODO: semaphore unsignaling
	// TODO: mesh loading
	// TODO: material management
	// TODO: shader management
// TODO: render_pass restructuring
// // TODO: Implement Better Destruction -
// TODO: Implement Image Loading
// TODO: Implement DearImGui


int main(int count, char** args) {
	freopen("output.log", "w", stdout);
	
	// std::vector<void*> data = {};
	
	// VkBuffer bffer;
	// VkSwapchainKHR swakp;
	
	// data.push_back(swakp);
	// data.push_back(bffer);
	
	// dloggln(sizeof(VkBuffer));
	// exit(0);
	// VkBuffer

	lucy::engine::initialize();
	lucy::engine::mainloop();
	lucy::engine::destroy();
}