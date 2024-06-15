#include "lucyvk/functions.h"
#include "lucyvk/create_info.h"
#include "lucyvk/shaders.h"

#include "lucyio/logger.h"

#include "frame.h"
#include "lucyvk/synchronization.h"

void lvk_frame::initialize(VkDevice device) {
	lvk_create_semaphore(device, &present_semaphore);
	lvk_create_semaphore(device, &render_semaphore);
	
	
}
