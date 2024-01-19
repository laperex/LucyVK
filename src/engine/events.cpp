#include "events.h"
#include "util/logger.h"
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

	for (int i = 0; i < SDL_PollEvent(&event); i++) {
		for (auto& function: event_function_array) {
			function(&event);
		}
		
		if (event.type == SDL_QUIT) {
			is_quit = true;
		}
		
		if (event.type == SDL_KEYUP) {
			pressed_keys.insert(event.key.keysym.scancode);
			
			if (key_chord.size() == 0 || key_chord.back() != event.key.keysym.scancode) {
				key_chord.push_back(event.key.keysym.scancode);
			}
		}

		if (event.type == SDL_KEYDOWN) {
			if (pressed_keys.count(event.key.keysym.scancode)) {
				if (!toggled_keys.contains(event.key.keysym.scancode)) {
						toggled_keys.insert(event.key.keysym.scancode);
				} else {
					toggled_keys.erase(event.key.keysym.scancode);
				}
			}
			key_chord.clear();
			pressed_keys.erase(event.key.keysym.scancode);
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
	return pressed_keys.contains(scancode);
}

bool lucy::events::key_toggled(SDL_Scancode scancode) {
	return toggled_keys.contains(scancode);
}

bool lucy::events::key_toggled(const std::vector<SDL_Scancode> scancode) {
	for (auto& key: scancode) {
		if (!toggled_keys.contains(key)) {
			return false;
		}
	}
	
	return true;
}

bool lucy::events::key_pressed(const std::vector<SDL_Scancode> scancode) {
	for (auto& key: scancode) {
		if (!toggled_keys.contains(key)) {
			return false;
		}
	}
	
	return true;
}
