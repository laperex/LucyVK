#include "Events.h"
// #include <imgui_impl_sdl.h>
#include <iostream>

void lucy::Events::Init() {
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
}

void lucy::Events::AddFunction(event_func func) {
	for (auto fn: self->event_func_array)
		assert(func != fn);

	self->event_func_array.push_back(func);
}

void lucy::Events::Update() {
	lucy::Events::IsQuittable() = lucy::Events::IsKeyPressed(SDL_SCANCODE_ESCAPE);
	
	if (!self->is_init) {
		Init();
		self->is_init = true;
	}

	if (self->is_window_resized != 0 && self->is_window_resized < 3) {
		self->is_window_resized++;
	} else self->is_window_resized = 0;

	self->scrolldown = false;
	self->scrollup = false;

	self->is_window_moved = false;
	self->is_file_dropped = false;

	self->dropfilename = "";
	self->drag_begin = false;
	static bool drag_end;
	static int drag_count;

	if (drag_count > 1) {
		self->payload_data = nullptr;
		self->payload_type = "";
		drag_end = false;
	}

	if (drag_end) {
		drag_count++;
	} else {
		drag_count = 0;
	}

	while (SDL_PollEvent(&self->event)) {
		for (auto func: self->event_func_array) {
			func(self->event);
		}

		if (self->event.type == SDL_QUIT)
			self->is_quit = true;

		if (self->event.type == SDL_MOUSEMOTION) {
			self->relmousepos.x += self->event.motion.xrel;
			self->relmousepos.y += self->event.motion.yrel;
			
			self->relmouseoffset.x = self->event.motion.xrel;
			self->relmouseoffset.y = self->event.motion.yrel;

			self->mousepos.x = self->event.motion.x;
			self->mousepos.y = self->event.motion.y;
		}
		if (self->event.type == SDL_KEYDOWN) {
			self->pressed_keys.insert(self->event.key.keysym.scancode);
			if (self->key_chord.size() == 0)
				self->key_chord.push_back(self->event.key.keysym.scancode);
			else if (self->key_chord.back() != self->event.key.keysym.scancode)
				self->key_chord.push_back(self->event.key.keysym.scancode);
		}
		if (self->event.type == SDL_KEYUP) {
			if (self->pressed_keys.count(self->event.key.keysym.scancode)) {
				if (!self->toggled_keys.contains(self->event.key.keysym.scancode))
					self->toggled_keys.insert(self->event.key.keysym.scancode);
				else
					self->toggled_keys.erase(self->event.key.keysym.scancode);
			}
			self->key_chord.clear();
			self->pressed_keys.erase(self->event.key.keysym.scancode);
		}
		if (self->event.type == SDL_MOUSEBUTTONDOWN) {
			self->pressed_buttons.insert(self->event.button.button);
			if (self->event.button.button == SDL_BUTTON_LEFT) {
				self->drag_begin = true;
			}
		}
		if (self->event.type == SDL_MOUSEBUTTONUP) {
			self->pressed_buttons.erase(self->event.button.button);
			if (self->event.button.button == SDL_BUTTON_LEFT) {
				drag_end = true;
			}
		}
		if (self->event.type == SDL_MOUSEWHEEL) {
			self->scrollup = (self->event.wheel.y > 0);
			self->scrolldown = (self->event.wheel.y < 0);
		}
		if (self->event.type == SDL_DROPFILE) {
			self->dropfilename = std::string(self->event.drop.file);
			self->is_file_dropped = true;
		}
		if (self->event.type == SDL_WINDOWEVENT) {
			if (self->event.window.event == SDL_WINDOWEVENT_RESIZED) {
				self->is_window_resized = true;
				self->window_resize_size.x = self->event.window.data1;
				self->window_resize_size.y = self->event.window.data2;
			}
			if (self->event.window.event == SDL_WINDOWEVENT_MOVED) {
				self->is_window_moved = true;
				self->window_move_position.x = self->event.window.data1;
				self->window_move_position.y = self->event.window.data2;
			}
			if (self->event.window.event == SDL_WINDOWEVENT_ENTER) {
				self->is_window_mouse_focus = true;
			}
			if (self->event.window.event == SDL_WINDOWEVENT_LEAVE) {
				self->is_window_mouse_focus = false;
			}
			if (self->event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
				self->is_window_keyboard_focus = true;
			}
			if (self->event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
				self->is_window_keyboard_focus = false;
			}
		}
	}
}

bool lucy::Events::IsKeyPressed(SDL_Scancode scancode) {
	return self->pressed_keys.contains(scancode);
}

bool lucy::Events::IsKeyPressed(const std::vector<SDL_Scancode>& scancodes) {
	for (int i = 0; i < scancodes.size(); i++) {
		if (!self->pressed_keys.contains(scancodes[i]))
			return false;
	}

	return true;
}

bool lucy::Events::IsKeyToggled(SDL_Scancode scancode) {
	return self->toggled_keys.contains(scancode);
}

bool lucy::Events::IsKeyChord(const std::vector<SDL_Scancode>& key_chord) {
	if (self->key_chord.size() < key_chord.size()) return false;

	for (int i = 0; i < self->key_chord.size(); i++) {
		if (self->key_chord[i] != key_chord[i])
			return false;
	}

	return true;
}

bool& lucy::Events::IsQuittable() {
	return self->is_quit;
}

const glm::vec2& lucy::Events::GetWindowSize() {
	return self->window_resize_size;
}

const glm::vec2& lucy::Events::GetWindowPosition() {
	return self->window_move_position;
}

bool lucy::Events::IsWindowResized() {
	return (self->is_window_resized > 0);
}

bool lucy::Events::IsWindowMoved() {
	return self->is_window_moved;
}

bool lucy::Events::IsWindowMouseFocused() {
	return self->is_window_mouse_focus;
}

bool lucy::Events::IsWindowKeyboardFocused() {
	return self->is_window_keyboard_focus;
}

bool lucy::Events::IsButtonPressed(unsigned int button) {
	return (self->pressed_buttons.contains(button));
}

bool lucy::Events::IsButtonPressedAny() {
	return (self->pressed_buttons.size() > 0);
}

bool lucy::Events::IsButtonToggled(unsigned int button) {
	return self->toggled_buttons.contains(button);
}

bool lucy::Events::IsMouseScrolling() {
	return (self->scrolldown || self->scrollup);
}

bool lucy::Events::IsMouseScrollingUp() {
	return self->scrollup;
}

bool lucy::Events::IsMouseScrollingDown() {
	return self->scrolldown;
}

// const glm::vec3& lucy::Events::Get_MainWindow_CursorPosNormalized() {
// 	auto& window = registry.store<Window>();
// 	return GetCursorPosNormalized(0, 0, window.size.x, window.size.y);
// }

const glm::vec3& lucy::Events::GetCursorPos() {
	return self->mousepos;
}

const glm::vec3 lucy::Events::GetCursorPosNormalized(float posx, float posy, float width, float height) {
	return glm::vec3 {
		((self->mousepos.x - posx) / (width * 0.5)) - 1.0,
		1.0 - ((self->mousepos.y - posy) / (height * 0.5)),
		0
	};
}

// const glm::vec3& lucy::Events::Get_MainWindow_Relative_CursorPosNormalized() {
// 	auto& window = registry.store<Window>();
// 	return GetRelCursorPosNormalized(0, 0, window.size.x, window.size.y);
// }

const glm::vec3& lucy::Events::GetRelCursorPos() {
	return self->relmousepos;
}

const glm::vec3 lucy::Events::GetRelCursorPosNormalized(float posx, float posy, float width, float height) {
	return glm::vec3 {
		((self->relmousepos.x - posx) / (width * 0.5)) - 1.0,
		1.0 - ((self->relmousepos.y - posy) / (height * 0.5)),
		0
	};
}

const glm::vec3& lucy::Events::GetRelCursorOffset() {
	return self->relmouseoffset;
}

const std::string& lucy::Events::GetDroppedFile() {
	return self->dropfilename;
}

bool lucy::Events::IsFileDropped() {
	return self->is_file_dropped;
}

void lucy::Events::SetPayload(void* data, const std::string& type) {
	self->payload_data = data;
	self->payload_type = type;
}

bool lucy::Events::IsPayloadPresent() {
	return (self->payload_data != nullptr);
}

void* lucy::Events::GetPayloadData() {
	return self->payload_data;
}

std::string lucy::Events::GetPayloadType() {
	return self->payload_type;
}

SDL_Event& lucy::Events::GetEvent() {
	return self->event;
}
