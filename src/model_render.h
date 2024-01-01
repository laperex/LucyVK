#pragma once

#include <cstdint>

class model_render {
	void init(const char* filename);
	void loop(float dt, uint32_t frame_number);
};