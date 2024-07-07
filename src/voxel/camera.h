#pragma once

#include "lucy/system.h"
#include "lucylm/math.hpp"

namespace lucy {
	class camera: public system_template {
		glm::mat4 projection;
		glm::mat4 view;

		glm::vec3 rotation = { 0, 0, 0 };

		glm::vec3 position = { 0.0, 0.0, 0.0 };
		glm::vec3 offset = { 0.0, 0.0, 0.0 };
		glm::vec3 world_front = { 0.0, 0.0, -1.0 };
		glm::vec3 front = world_front;
		glm::vec3 world_up = { 0.0, 1.0, 0.0 };
		glm::vec3 up = world_up;

		int width, height;
		int posx, posy;
		int lastx, lasty;
		bool first_mouse = true;
		float fov = 45.0f;
		float c_near = 0.01f, c_far = 1000.0f;
		float sensitivity = 0.5f;

		bool enable = true;

	public:
		void initialization();
		void update(double dt);
		void destroy();
	};
}