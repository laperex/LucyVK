#include "engine/engine.h"
#include "world/world.h"

int main(int count, char** args) {
	lucy::engine engine = {};

	lucy::world world = { engine };
	
	auto initialization_function = [&](){
		world.initialize();
	};
	
	auto mainloop_function = [&](double dt){
		world.update(dt);
	};

	engine.initialize(initialization_function);
	
	engine.mainloop(mainloop_function);
	
	engine.destroy();
}