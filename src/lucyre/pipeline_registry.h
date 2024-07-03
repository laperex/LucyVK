#pragma once

#include "lucyvk/device.h"
#include <map>
#include <string>

namespace lre {
	struct pipeline_registry {
		std::map<std::string, lvk_pipeline> registry;
		
		void create_pipeline();
	};
}
