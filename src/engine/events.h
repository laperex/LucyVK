#pragma once

#include <SDL_events.h>
#include <functional>
#include <set>
#include <vector>

namespace lucy {
	class events {
		bool is_quit;
		bool is_init;
		bool is_resized;
		bool is_key_pressed;
		bool is_key_toggled;
		
		std::set<SDL_Scancode> pressed_keys;
		std::set<SDL_Scancode> toggled_keys;
		std::vector<SDL_Scancode> key_chord;

		std::vector<std::function<void(const SDL_Event*)>> event_function_array = {};

	public:
		typedef decltype(*event_function_array.data()) function;

		void add(function event_function);

		void initialize();
		void update();

		bool& quit();
		const bool& resized();
		
		bool key_pressed(SDL_Scancode scancode);
		bool key_toggled(SDL_Scancode scancode);
		
		bool key_pressed(const std::vector<SDL_Scancode> scancode);
		bool key_toggled(const std::vector<SDL_Scancode> scancode);
	};
}