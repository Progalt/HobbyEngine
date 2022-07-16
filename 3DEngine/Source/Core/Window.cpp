
#include "Window.h"

#include "GlobalState.h"

#include <SDL2/SDL_syswm.h>

Window::~Window()
{
	if (mWindow)
	{
		SDL_DestroyWindow(mWindow);

		GlobalState::GetInstance().ShutdownSDL();
	}
}

void Window::Create(const std::string& title, const uint32_t width, const uint32_t height, WindowContext contextType)
{

	GlobalState::GetInstance().InitSDL();

	int flags = SDL_WINDOW_SHOWN;

	if (contextType == WindowContext::Vulkan)
		flags |= SDL_WINDOW_VULKAN;


	mWindow = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, flags);

	if (!mWindow)
	{

	}

	mWidth = width;
	mHeight = height;

	mIsOpen = true;
		
}

void Window::PollEvents()
{
	SDL_Event ev;

	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
		case SDL_QUIT:
			mIsOpen = false;
			break;
		}
	}
}

HWND Window::GetRawWindow()
{
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(mWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	return hwnd;
}