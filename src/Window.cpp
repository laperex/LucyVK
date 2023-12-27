#include "Window.h"

lucy::Window::Window() {
	this->pos = { 0, 0 };
	this->size = { 1920, 1080 };

	this->size *= 0.7;

	this->title = "Lucy Framework V7";
	
	this->sdl_window = nullptr;
}

lucy::Window::~Window() {
	Destroy();
}

void lucy::Window::InitWindow() {
	assert(this->sdl_window == nullptr);

	SDL_Init(SDL_INIT_VIDEO);

	int off_x = 0;
	int off_y = 0;

	// int displays = SDL_GetNumVideoDisplays();
	// assert(displays > 1);

	// std::vector<SDL_Rect> displayBounds;
	// for(int i = 0; i < displays; i++) {
	// 	displayBounds.push_back(SDL_Rect());
	// 	SDL_GetDisplayBounds(i, &displayBounds.back());
	// }

	// off_x = displayBounds[1].x;
	// off_y = displayBounds[1].y;

	this->sdl_window = SDL_CreateWindow(this->title.c_str(), this->pos.x + off_x, this->pos.y + off_y, this->size.x, this->size.y, this->flags);

	assert(this->sdl_window);

	// this->sdl_glcontext = SDL_GL_CreateContext(this->sdl_window);
	// this->sdl_glcontext
}

void lucy::Window::InitWindow(const char* title, int pos_x, int pos_y, int width, int height, uint32_t flags) {
	this->pos.x = pos_x;
	this->pos.y = pos_y;
	this->size.x = width;
	this->size.y = height;
	this->title = std::string(title);
	this->flags |= flags;

	InitWindow();
}

void lucy::Window::SwapWindow() {
	SDL_GL_SwapWindow(this->sdl_window);
}

void lucy::Window::Destroy() {
	// if (this->framebuffer != nullptr) {
	// 	delete this->framebuffer;
	// 	this->framebuffer = nullptr;
	// }

	SDL_DestroyWindow(this->sdl_window);
}

void lucy::Window::SetCurrent() {
	SDL_GL_MakeCurrent(this->sdl_window, this->sdl_glcontext);
}

void lucy::Window::ResetSize() {
	this->size = GetActualSize();
}

void lucy::Window::ResetPosition() {
	this->pos = GetActualPosition();
}

void lucy::Window::Hide() {
	if (!this->hidden)
		SDL_HideWindow(this->sdl_window);
	this->hidden = true;
}

void lucy::Window::Show() {
	if (this->hidden)
		SDL_ShowWindow(this->sdl_window);
	this->hidden = false;
}

glm::ivec2 lucy::Window::GetActualSize() {
	int w, h;
	SDL_GetWindowSize(this->sdl_window, &w, &h);
	return { w, h };
}

glm::ivec2 lucy::Window::GetActualPosition() {
	int x, y;
	SDL_GetWindowPosition(this->sdl_window, &x, &y);
	return { x, y };
}
