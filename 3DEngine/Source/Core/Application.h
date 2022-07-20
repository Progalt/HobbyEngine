#pragma once

#include "Window.h"
#include "Profiler.h"

#include "Input.h"


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
		window.Create(info.title, info.width, info.height, WindowContext::Vulkan);

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

			profiler.BeginProfiling("Frame");

			window.PollEvents();



			Update();
			Render();

			profiler.EndProfiling("Frame");
		}

		Destroy();
	}

	Profiler profiler;

	Window window;

	struct
	{
		float delta;
	} time;

};