#include "engine.h"
#include "Events.h"

#include <chrono>

void lucy::engine::initialize() {
	window.initialize("lucy", { 50, 50 }, { 1920, 1080 });
	
	renderer.initialization(&window);
}

void lucy::engine::mainloop() {
	double dt = 0;
	uint32_t frame_number = 0;
	while (!lucy::Events::IsQuittable()) {
		const auto& start_time = std::chrono::high_resolution_clock::now();

		lucy::Events::Update();

		frame_number += 1;
		const auto& end_time = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<double, std::ratio<1, 60>>(end_time - start_time).count();
	}
}
