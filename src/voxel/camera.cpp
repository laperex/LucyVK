#include "camera.h"
#include "lucyio/logger.h"

glm::vec2 last_size = { 0, 0 };
glm::vec2 last_pos = { 0, 0 };
float scrollspeed = 0.75 * 3;
bool toggle = false;

glm::vec3 delta;
glm::vec3 initpos;

void lucy::camera::initialization() {
	position = { 0, 0, -10 };
	c_near = 0.1;
	c_far = 2000.f;
}

void lucy::camera::update(double dt) {
	this->width = _window->size().x;
	this->height = _window->size().y;
	
	if (this->width != last_size.x || this->height != last_size.y || this->posx != last_pos.x || this->posy != last_pos.y) {
		last_size.x = this->width;
		last_size.y = this->height;
		last_pos.x = this->posx;
		last_pos.y = this->posy;

		this->lastx = this->width / 2;
		this->lasty = this->height / 2;

		this->first_mouse = true;

		this->projection = glm::perspective(glm::radians(this->fov), (float)this->width / this->height, this->c_near, this->c_far);
	}

	if (!enable) {
		this->first_mouse = true;
		return;
	}

	auto norm_cursor_pos = _events->cursor_position_normalized(this->posx, this->posy, this->width, this->height);
	auto cursor_pos = _events->cursor_position();

	if (_events->button_pressed(SDL_BUTTON_RIGHT)) {
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

	if (_events->scroll_up())
		this->position += this->front * float(scrollspeed * dt);
	if (_events->scroll_down())
		this->position -= this->front * float(scrollspeed * dt);

	dloggln(glm::to_string(position));

	float distance = glm::length(this->position);
	scrollspeed = distance / 10;
	this->position = distance * -this->front;

	static bool click_toggle = true;
	if (_events->button_pressed(SDL_BUTTON_LEFT) && _events->key_pressed(SDL_SCANCODE_LALT)/*  && click_toggle */) {
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
	click_toggle = !_events->button_pressed(SDL_BUTTON_LEFT);

	this->view = glm::lookAt(this->position + this->offset + delta, this->position + this->offset + delta + this->front, this->up);

	if (toggle && !_events->button_pressed(SDL_BUTTON_LEFT) && _events->key_pressed(SDL_SCANCODE_LALT)) {
		this->offset += delta;
		toggle = false;
		delta = glm::vec3();
	}

	_renderer->set_projection(projection);
	_renderer->set_view(view);
	_renderer->set_model(glm::mat4(1.0f));
}

void lucy::camera::destroy() {
	
}

