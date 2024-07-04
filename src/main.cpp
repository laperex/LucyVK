// #include "lucy/engine.h"
#include <lucy/engine.h>


int main(int count, char** args) {
	lucy::engine::initialize();
	lucy::engine::mainloop();
	lucy::engine::destroy();
}