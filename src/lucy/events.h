#pragma once

#include <SDL_events.h>
#include <functional>
#include <set>
#include <vector>
#include "lucylm/math.hpp"

namespace lucy {
	class events {
		bool is_quit = false;
		bool is_init = false;
		bool is_resized = false;
		bool is_key_pressed = false;
		bool is_key_toggled = false;

		float mouse_wheel_scroll = 0;

		glm::ivec2 _cursor_pos = {};
		glm::ivec2 _cursor_pos_rel = {};
		glm::ivec2 _cursor_offset_rel = {};

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
		
		bool button_pressed(const uint32_t button);
		bool scroll_up();
		bool scroll_down();

		glm::ivec2 cursor_position();
		glm::vec2 cursor_position_normalized(float posx, float posy, float width, float height);
	};
}