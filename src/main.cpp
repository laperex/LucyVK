// #include "lucy/engine.h"
#include "lucyio/logger.h"
#include <lucy/engine.h>
#include <stdio.h>
#include <unistd.h>
#include <vulkan/vulkan_core.h>

#include "lucytl/linked_list.h"
#include "lucytl/static_list.h"
#include "lucytl/array.h"
#include "voxel/camera.h"
#include "voxel/world.h"


// // TODO: dynamic viewport
// TODO: renderer restructuring
	// // TODO: semaphore unsignaling
	//! TODO: GLTF loading
	// TODO: material management
	// TODO: shader management
// // TODO: memory handles
// // TODO: render_pass restructuring
// // TODO: better memory management
// // TODO: Implement Better Destruction -
// TODO: Implement Better Image Loading
// // TODO: Implement DearImGui

void a() {
	printf("sdkjansd");
}

typedef void(*NULL_DU)();

int main(int count, char** args) {
	lucy::engine engine;

	engine.add<lucy::camera>();
	engine.add<lucy::world>();

	engine.initialize();
	engine.mainloop();
	engine.destroy();
}
