#pragma once

#include "Window.h"

#include "Input.h"
#include "../Threading/JobSystem.h"
#include "GlobalState.h"

class Application
{
public:

	struct StartInfo
	{
		std::string title;
		uint32_t width, height;
	};

	virtual void Start() {}
	virtual void Update() {}
	virtual void Render() { }
	virtual void Destroy() {}

	void Run(const StartInfo& info)
	{
		try
		{
			//JobSystem::Init();

			window.Create(info.title, info.width, info.height, WindowContext::Vulkan);

			GlobalState::GetInstance().width = info.width;
			GlobalState::GetInstance().height = info.height;

			EventManager::GetInstance().Init();

			EventManager::GetInstance().Attach(&input, EventType::KeyboardInput);
			EventManager::GetInstance().Attach(&input, EventType::MouseButton);

			Start();

			Uint64 NOW = SDL_GetPerformanceCounter();
			Uint64 LAST = 0;

			while (window.IsOpen())
			{
				LAST = NOW;
				NOW = SDL_GetPerformanceCounter();

				time.delta = (float)((NOW - LAST) * 1000 / (float)SDL_GetPerformanceFrequency()) * 0.001f;


				window.PollEvents();



				Update();
				Render();

			}

			Destroy();
		}
		catch(std::exception& ex)
		{
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", ex.what(), NULL);
		}
	}


	Window window;

	struct
	{
		float delta;
	} time;

};