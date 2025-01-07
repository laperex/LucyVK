#pragma once

#include "lucy/engine.h"

namespace lucy {
	class world: public system_template {
	public:
		void initialization();
		void update(double dt);
		void destroy();
	};
}