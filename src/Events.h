#pragma once

#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_events.h>

#include <lucyvm/lvm_vector.h>

#include <string>
#include <set>
#include <vector>

namespace lucy {
	class Events {
		bool is_quit;
		bool is_init = false;

		typedef void(*event_func)(SDL_Event&);

		std::vector<event_func> event_func_array;

		SDL_Event event;

		std::set<SDL_Scancode> pressed_keys;
		std::set<SDL_Scancode> toggled_keys;
		std::vector<SDL_Scancode> key_chord;

		std::set<unsigned int> pressed_buttons;
		std::set<unsigned int> toggled_buttons;
		glm::vec3 mousepos;
		glm::vec3 relmousepos;
		glm::vec3 relmouseoffset;

		bool scrollup;
		bool scrolldown;

		std::string dropfilename;
		bool is_file_dropped = false;

		glm::vec2 window_resize_size;
		glm::vec2 window_move_position;
		int is_window_resized;
		bool is_window_moved;
		bool is_window_mouse_focus;
		bool is_window_keyboard_focus;

		void* payload_data = nullptr;
		std::string payload_type;

		bool drag_begin;

	private:
		static void Init();

		#define self Instance()
		static Events* Instance() {
			static Events instance;
			return &instance;
		}

	public:
		static void AddFunction(event_func func);
		static void Update();

		static bool IsKeyPressed(SDL_Scancode scancode);
		static bool IsKeyPressedAny(SDL_Scancode scancode);
		static bool IsKeyPressed(const std::vector<SDL_Scancode>& scancodes);
		static bool IsKeyToggled(SDL_Scancode scancode);
		static bool IsKeyChord(const std::vector<SDL_Scancode>& scancodes);

		static bool& IsQuittable();
		static const glm::vec2& GetWindowSize();
		static const glm::vec2& GetWindowPosition();
		static bool IsWindowResized();
		static bool IsWindowMoved();
		static bool IsWindowMouseFocused();
		static bool IsWindowKeyboardFocused();

		static bool IsButtonPressed(unsigned int button);
		static bool IsButtonPressedAny();
		static bool IsButtonToggled(unsigned int button);

		static bool IsMouseScrolling();
		static bool IsMouseScrollingUp();
		static bool IsMouseScrollingDown();

		static const glm::vec3& GetCursorPos();
		static const glm::vec3 GetCursorPosNormalized(float posx, float posy, float width, float height);
		static const glm::vec3& GetRelCursorPos();
		static const glm::vec3 GetRelCursorPosNormalized(float posx, float posy, float width, float height);
		static const glm::vec3& GetRelCursorOffset();

		// static const glm::vec3& Get_MainWindow_CursorPosNormalized();
		// static const glm::vec3& Get_MainWindow_Relative_CursorPosNormalized();

		static const std::string& GetDroppedFile();
		static bool IsFileDropped();

		static void SetPayload(void* data, const std::string& type);
		static bool IsPayloadPresent();
		static void* GetPayloadData();
		static std::string GetPayloadType();

		static SDL_Event& GetEvent();
	};
}