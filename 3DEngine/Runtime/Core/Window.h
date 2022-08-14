#pragma once

#include "Platform.h"

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <SDL2/SDL_vulkan.h>
#include <vector>
#include <functional>

enum class WindowContext
{
	Vulkan
};

class Window
{
public:

	~Window();

	void Create(const std::string& title, const uint32_t width, const uint32_t height, WindowContext contextType);

	void PollEvents();

	const bool IsOpen() const { return mIsOpen; }

	const uint32_t GetWidth() const { return mWidth; }
	const uint32_t GetHeight() const { return mHeight; }

#ifdef _WIN32
	HWND GetRawWindow();
#endif

	VkSurfaceKHR CreateSurface(VkInstance instance);

	SDL_Window* GetWindow() { return mWindow; }

	void AddEventCallback(std::function<void(SDL_Event*)> callback)
	{
		mEventCallbacks.push_back(callback);
	}



private:

	std::vector<std::function<void(SDL_Event*)>> mEventCallbacks;

	SDL_Window* mWindow;

	uint32_t mWidth, mHeight;

	bool mIsOpen;
};