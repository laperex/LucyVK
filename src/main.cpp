#include "engine/engine.h"

int main(int count, char** args) {
	lucy::engine engine;
	
	engine.initialize();
	
	engine.mainloop();
	
	engine.destroy();
}