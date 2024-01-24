#pragma once

#include "engine/engine.h"
#include "engine/renderer.h"
#include "engine/events.h"
#include "world/camera.h"

namespace lucy {
	struct world {
		events* _events;
		renderer* _renderer;
		
		lucy::camera perspective_camera;

		world(engine& engine);

		void initialize();
		void update(double dt);
	};
}