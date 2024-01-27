#pragma once

#include "lucy/engine.h"
#include "lucyre/renderer.h"
#include "lucy/events.h"
#include "voxel/camera.h"

namespace lucy {
	struct world {
		static void initialize();
		static void update(double dt);
	};
}