// #include "lucy/engine.h"
#include <lucy/engine.h>


//* TODO: Implement Better Destruction -
//* TODO: Implement Image Loading
//* TODO: Implement DearImGui


int main(int count, char** args) {
	lucy::engine::initialize();
	lucy::engine::mainloop();
	lucy::engine::destroy();
}