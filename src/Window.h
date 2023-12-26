#pragma once

#include <lucyvm/lvm_vector.h>

#include <string>
#include <SDL2/SDL.h>

namespace lucy {
	struct Window {
		glm::vec2 pos = { 50, 50 };
		glm::vec2 size = { 1366, 768 };

		uint32_t flags = SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN;

		std::string title = "Lucy-Framework";
		SDL_Window* sdl_window = nullptr;
		SDL_GLContext sdl_glcontext;

		bool hidden = false;
		bool enable_framebuffer = true;

		Window();
		~Window();

		void InitWindow();
		void InitWindow(const char* title, int pos_x, int pos_y, int width, int height, uint32_t flags);
		void SwapWindow();
		void Destroy();
		void SetCurrent();
		void ResetSize();
		void ResetPosition();
		void Hide();
		void Show();

		glm::ivec2 GetActualSize();
		glm::ivec2 GetActualPosition();
	};
}