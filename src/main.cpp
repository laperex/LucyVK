#include "lucy/engine.h"
#include "voxel/world.h"

int main(int count, char** args) {
	lucy::engine::initialize();
	
	lucy::engine::mainloop();
	
	lucy::engine::destroy();
}