#include "events.h"
#include "lucyio/logger.h"
#include <stdexcept>

void lucy::events::add(function event_function) {
	if (event_function == nullptr) {
		throw std::runtime_error("Event Function Cannot be nullptr");
	}

	event_function_array.push_back(event_function);
}

void lucy::events::initialize() {
	
}

void lucy::events::update() {
	SDL_Event event;
	
	is_resized = false;

	for (int i = 0; i < SDL_PollEvent(&event); i++) {
		for (auto& function: event_function_array) {
			function(&event);
		}
		
		switch (event.type) {
			case SDL_QUIT:
				is_quit = true;
				break;
			
			case SDL_MOUSEMOTION:
				mouse_position.x = event.motion.x;
				mouse_position.y = event.motion.y;
				
				mouse_offset_relative.x = event.motion.xrel;
				mouse_offset_relative.y = event.motion.yrel;
				
				mouse_position_relative.x += event.motion.xrel;
				mouse_position_relative.y += event.motion.yrel;
				break;
			
			case SDL_MOUSEBUTTONDOWN:
				mouse_pressed.insert(event.button.button);
				break;
			case SDL_MOUSEBUTTONUP:
				mouse_pressed.erase(event.button.button);
				break;
			case SDL_MOUSEWHEEL:
				mouse_wheel_scroll = event.wheel.y;
				break;
			
			case SDL_KEYUP:
				keyboard_pressed.insert(event.key.keysym.scancode);
				
				if (key_chord.size() == 0 || key_chord.back() != event.key.keysym.scancode) {
					key_chord.push_back(event.key.keysym.scancode);
				}
				break;

			case SDL_KEYDOWN:
				if (keyboard_pressed.count(event.key.keysym.scancode)) {
					if (!keyboard_toggled.contains(event.key.keysym.scancode)) {
							keyboard_toggled.insert(event.key.keysym.scancode);
					} else {
						keyboard_toggled.erase(event.key.keysym.scancode);
					}
				}
				key_chord.clear();
				keyboard_pressed.erase(event.key.keysym.scancode);
				break;
			
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						is_resized = true;
				}
				break;
		}
	}
}

bool& lucy::events::quit() {
	return is_quit;
}

const bool& lucy::events::resized() {
	return is_resized;
}

bool lucy::events::key_pressed(SDL_Scancode scancode) {
	return keyboard_pressed.contains(scancode);
}

bool lucy::events::key_toggled(SDL_Scancode scancode) {
	return keyboard_toggled.contains(scancode);
}

bool lucy::events::key_toggled(const std::vector<SDL_Scancode> scancode) {
	for (auto& key: scancode) {
		if (!keyboard_toggled.contains(key)) {
			return false;
		}
	}
	
	return true;
}

bool lucy::events::key_pressed(const std::vector<SDL_Scancode> scancode) {
	for (auto& key: scancode) {
		if (!keyboard_toggled.contains(key)) {
			return false;
		}
	}
	
	return true;
}
