#pragma once

#include <SDL_events.h>
#include <functional>
#include <set>
#include <vector>
#include <math/math.hpp>

namespace lucy {
	class events {
		bool is_quit = false;
		bool is_init = false;
		bool is_resized = false;
		bool is_key_pressed = false;
		bool is_key_toggled = false;
		

		float mouse_wheel_scroll = 0;
		
		glm::ivec2 mouse_position = {};
		glm::ivec2 mouse_position_relative = {};
		glm::ivec2 mouse_offset_relative = {};

		std::set<uint32_t> mouse_pressed = {};
		std::set<uint32_t> mouse_toggled = {};


		std::vector<SDL_Scancode> key_chord = {};

		std::set<SDL_Scancode> keyboard_pressed = {};
		std::set<SDL_Scancode> keyboard_toggled = {};

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