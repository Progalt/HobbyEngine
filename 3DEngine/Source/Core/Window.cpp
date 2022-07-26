
#include "Window.h"

#include "GlobalState.h"

#include <SDL2/SDL_syswm.h>

#include "EventManager.h"

#include "Log.h"

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
		Log::Error("SDL", "Failed to Create Window: %s", SDL_GetError());
	}

	mWidth = width;
	mHeight = height;

	mIsOpen = true;

	Log::Info("SDL", "Created Window");
		
}

void Window::PollEvents()
{
	SDL_Event ev;

	while (SDL_PollEvent(&ev))
	{
		for (auto callback : mEventCallbacks)
			callback(&ev);

		switch (ev.type)
		{
		case SDL_QUIT:
			mIsOpen = false;
			break;
		case SDL_WINDOWEVENT:

			switch (ev.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:

				std::shared_ptr<WindowResizeEvent> resizeEvent = std::make_shared<WindowResizeEvent>(EventType::WindowResize);

				resizeEvent->newWidth = ev.window.data1;
				resizeEvent->newHeight = ev.window.data2;

				mWidth = ev.window.data1;
				mHeight = ev.window.data2;

				EventManager::GetInstance().SendEvent(resizeEvent);

				break;
			}

			break;

		case SDL_KEYDOWN:
		{
			std::shared_ptr<KeyboardEvent> keyboardEvent = std::make_shared<KeyboardEvent>(EventType::KeyboardInput);
			keyboardEvent->buttonState = ButtonState::Pressed;
			keyboardEvent->keycode = TranslateVirtualKey(ev.key.keysym.sym);

			EventManager::GetInstance().SendEvent(keyboardEvent);
		}
		break;
		case SDL_KEYUP:
		{
			std::shared_ptr<KeyboardEvent> keyboardEvent = std::make_shared<KeyboardEvent>(EventType::KeyboardInput);
			keyboardEvent->buttonState = ButtonState::Released;
			keyboardEvent->keycode = TranslateVirtualKey(ev.key.keysym.sym);

			EventManager::GetInstance().SendEvent(keyboardEvent);
		}
		case SDL_MOUSEMOTION:
		{
			static uint32_t prevmx = 0, prevmy = 0;
			std::shared_ptr<MouseMoveEvent> mouseMoveEvent = std::make_shared<MouseMoveEvent>(EventType::MouseMove);

			mouseMoveEvent->x = ev.motion.x;
			mouseMoveEvent->y = ev.motion.y;
			mouseMoveEvent->dx = mouseMoveEvent->x - prevmx;
			mouseMoveEvent->dy = mouseMoveEvent->y - prevmy;

			prevmx = ev.motion.x;
			prevmy = ev.motion.y;

			EventManager::GetInstance().SendEvent(mouseMoveEvent);
		}
		break;
		case SDL_MOUSEBUTTONDOWN:
		{
			std::shared_ptr<MouseButtonEvent> mouseButton = std::make_shared<MouseButtonEvent>(EventType::MouseButton);

			mouseButton->state = ButtonState::Pressed;
			mouseButton->button = (MouseButton)ev.button.button;

			EventManager::GetInstance().SendEvent(mouseButton);
		}
		break;
		case SDL_MOUSEBUTTONUP:
		{
			std::shared_ptr<MouseButtonEvent> mouseButton = std::make_shared<MouseButtonEvent>(EventType::MouseButton);

			mouseButton->state = ButtonState::Released;
			mouseButton->button = (MouseButton)ev.button.button;

			EventManager::GetInstance().SendEvent(mouseButton);
		}
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

VkSurfaceKHR Window::CreateSurface(VkInstance instance)
{
	VkSurfaceKHR surface;
	if (!SDL_Vulkan_CreateSurface(mWindow, instance, &surface))
	{
		Log::Error("SDL", "Failed to Create Vulkan Surface");
	}

	return surface;
}
