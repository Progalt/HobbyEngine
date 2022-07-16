#pragma once

#include "Window.h"

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

		Start();

		while (window.IsOpen())
		{
			window.PollEvents();

			Update();
			Render();
		}

		Destroy();
	}


	Window window;


};