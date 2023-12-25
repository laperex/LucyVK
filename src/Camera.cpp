#include "Camera.h"
#include "Events.h"
#include "util/logger.h"

glm::vec2 last_size = { 0, 0 };
glm::vec2 last_pos = { 0, 0 };
float scrollspeed = 0.75 * 3;
bool toggle = false;

glm::vec3 delta;
glm::vec3 initpos;

lucy::Camera::Camera() {
	// position = { 0, 0, 500 };
	position = { 0, 0, -10 };
	c_near = 0.1f;
	c_far = 200.0f;
}

void lucy::Camera::Update(double dt) {
	if (this->width != last_size.x || this->height != last_size.y || this->posx != last_pos.x || this->posy != last_pos.y) {
		last_size.x = this->width;
		last_size.y = this->height;
		last_pos.x = this->posx;
		last_pos.y = this->posy;

		this->lastx = this->width / 2;
		this->lasty = this->height / 2;

		this->first_mouse = true;

		this->projection = glm::perspective(glm::radians(this->fov), float(this->width) / float(this->height), this->c_near, this->c_far);
		// this->projection[1][1] *= -1;
	}

	if (!enable) {
		this->first_mouse = true;
		return;
	}

	auto norm_cursor_pos = Events::GetCursorPosNormalized(this->posx, this->posy, this->width, this->height);
	norm_cursor_pos.y = 1 - norm_cursor_pos.y;
	auto cursor_pos = Events::GetCursorPos();
	cursor_pos.y = this->height - cursor_pos.y;

	if (Events::IsButtonPressed(SDL_BUTTON_RIGHT)) {
		if (this->first_mouse) {
			this->lastx = cursor_pos.x;
			this->lasty = cursor_pos.y;
			this->first_mouse = false;
		}

		this->rotation.y -= (cursor_pos.x - this->lastx) * this->sensitivity;
		this->rotation.x += (this->lasty - cursor_pos.y) * this->sensitivity;

		this->lastx = cursor_pos.x;
		this->lasty = cursor_pos.y;
	} else {
		this->first_mouse = true;
	}

	const auto& quaternion = glm::quat(glm::radians(this->rotation));

	this->front = glm::normalize(quaternion * this->world_front);
	this->up = glm::normalize(quaternion * this->world_up);

	if (Events::IsMouseScrollingUp())
		this->position += this->front * float(scrollspeed * dt);
	if (Events::IsMouseScrollingDown())
		this->position -= this->front * float(scrollspeed * dt);

	float distance = glm::length(this->position);
	scrollspeed = distance / 10;
	this->position = distance * -this->front;

	static bool click_toggle = true;
	if (Events::IsButtonPressed(SDL_BUTTON_LEFT) && Events::IsKeyPressed(SDL_SCANCODE_LALT)/*  && click_toggle */) {
		glm::vec4 ray_clip = glm::vec4(norm_cursor_pos.x, norm_cursor_pos.y, -1.0, 1.0);
		glm::vec4 ray_eye = glm::inverse(this->projection) * ray_clip;
		ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
		glm::vec3 ray_wor = glm::inverse(this->view) * ray_eye;
		ray_wor = glm::normalize(ray_wor);

		float denom = glm::dot(this->front, ray_wor);
		float t = -glm::dot((this->front * distance), this->front) / denom;
		glm::vec3 pos = ray_wor * t + (this->offset + this->position);

		if (!toggle)
			initpos = pos;

		delta = pos - initpos;
		toggle = true;
	}
	click_toggle = !Events::IsButtonPressed(SDL_BUTTON_LEFT);

	this->view = glm::lookAt(this->position + this->offset + delta, this->position + this->offset + delta + this->front, this->up);

	if (toggle && !Events::IsButtonPressed(SDL_BUTTON_LEFT) && Events::IsKeyPressed(SDL_SCANCODE_LALT)) {
		this->offset += delta;
		toggle = false;
		delta = {};
	}
}

