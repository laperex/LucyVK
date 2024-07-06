// #include "lucy/engine.h"
#include <lucy/engine.h>
#include <stdio.h>


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
	// freopen("output.log", "a", stderr);

	lucy::engine::initialize();
	lucy::engine::mainloop();
	lucy::engine::destroy();
}