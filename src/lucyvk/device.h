#pragma once

#include "lucyvk/types.h"

namespace lucyvk {
	struct physicaldevice;

	struct LogicalDevice {
		Instance& _instance;

		LogicalDevice(Instance& _instance): _instance(_instance) {};
		void init();
	};
}
