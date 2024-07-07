#pragma once

#include "lucy/system.h"
#include "lucy/engine.h"
#include "lucyre/renderer.h"
#include "lucy/events.h"
#include "voxel/camera.h"

namespace lucy {
	class world: public system_template {
		void initialization();
		void update();
		void destroy();
	};
}