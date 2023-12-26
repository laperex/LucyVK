#include "lucyvm/lvm_function.h"

glm::mat4 lucy::projection_perspective(uint32_t width, uint32_t height, float fov, float near, float far) {
	return glm::perspective(glm::radians(fov), float(width)/float(height), near, far);
}
