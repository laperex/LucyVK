#include "camera.h"
#include "lucyio/logger.h"

void lucy::camera::initialization() {
	position = { 0, 0, -10 };
	c_near = 0.1;
	c_far = 2000.f;
}

void lucy::camera::update(double dt) {
	static bool toggle = false;

	window_size = _window->size();
	
	if (window_size != prev_window_size) {
		prev_window_size = window_size;
		prev_cursor_pos = window_size / 2;

		first_mouse = true;
		projection = glm::perspective(glm::radians(fov), (float)window_size.x / window_size.y, c_near, c_far);
	}

	if (!enable) {
		first_mouse = true;
		return;
	}

	auto norm_cursor_pos = _events->cursor_position_normalized(0, 0, window_size.x, window_size.y);
	auto cursor_pos = _events->cursor_position();

	if (_events->button_pressed(SDL_BUTTON_RIGHT)) {
		if (first_mouse) {
			prev_cursor_pos = cursor_pos;
			first_mouse = false;
		}

		rotation.y -= (cursor_pos.x - prev_cursor_pos.x) * sensitivity;
		rotation.x += (prev_cursor_pos.y - cursor_pos.y) * sensitivity;

		prev_cursor_pos.x = cursor_pos.x;
		prev_cursor_pos.y = cursor_pos.y;
	} else {
		first_mouse = true;
	}

	const auto& quaternion = glm::quat(glm::radians(rotation));

	front = glm::normalize(quaternion * world_front);
	up = glm::normalize(quaternion * world_up);


	if (_events->scroll_up())
		position += front * float(scrollspeed * dt);
	if (_events->scroll_down())
		position -= front * float(scrollspeed * dt);


	float distance = glm::length(position);
	scrollspeed = distance / 10;
	position = distance * -front;

	static glm::vec3 delta;
	static glm::vec3 initpos;

	static bool click_toggle = true;
	if (_events->button_pressed(SDL_BUTTON_LEFT) && _events->key_pressed(SDL_SCANCODE_LALT)/*  && click_toggle */) {
		glm::vec4 ray_clip = glm::vec4(norm_cursor_pos.x, norm_cursor_pos.y, -1.0, 1.0);
		glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
		ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
		glm::vec3 ray_wor = glm::inverse(view) * ray_eye;
		ray_wor = glm::normalize(ray_wor);

		float denom = glm::dot(front, ray_wor);
		float t = -glm::dot((front * distance), front) / denom;
		glm::vec3 pos = ray_wor * t + (offset + position);

		if (!toggle)
			initpos = pos;

		delta = pos - initpos;
		toggle = true;
	}
	click_toggle = !_events->button_pressed(SDL_BUTTON_LEFT);

	view = glm::lookAt(position + offset + delta, position + offset + delta + front, up);

	if (toggle && !_events->button_pressed(SDL_BUTTON_LEFT) && _events->key_pressed(SDL_SCANCODE_LALT)) {
		offset += delta;
		toggle = false;
		delta = glm::vec3();
	}
	

	_renderer->set_projection(projection);
	_renderer->set_view(view);
}

void lucy::camera::destroy() {
	
}

