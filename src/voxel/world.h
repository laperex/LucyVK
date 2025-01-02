#pragma once

#include "lucy/system.h"
#include "lucy/engine.h"
#include "lucyvk/renderer.h"
#include "lucy/events.h"
#include "voxel/camera.h"

namespace lucy {
	class world: public system_template {
	public:
		void initialization();
		void update(double dt);
		void destroy();
	};
}