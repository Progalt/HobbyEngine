

#include "Core/Application.h"
#include "Renderer/RenderManager.h"
#include "Core/Log.h"
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <imgui.h>

#include "Maths/Frustum.h"

#include "Resources/Model.h"
#include "Scene/Scene.h"

class App : public Application
{
public:
	void Start() override
	{
		RenderManagerCreateInfo rmInfo{};
		rmInfo.renderWidth = window.GetWidth();
		rmInfo.renderHeight = window.GetHeight();
		renderManager = RenderManager::Create(&window, rmInfo);

		ResourceManager::GetInstance().SetRenderManager(renderManager);
	}

	void Update() override
	{

	}

	void Render() override
	{

	}

	void Destroy() override
	{
		ResourceManager::GetInstance().Discard();

		RenderManager::Destroy(renderManager);
	}

	void ImGuiRender()
	{

	}

	RenderManager* renderManager;
};

int main(int argc, char* argv[])
{
	Application::StartInfo startInfo{};
	startInfo.width = 1280;
	startInfo.height = startInfo.width / 16 * 9;;
	startInfo.title = "Editor";

	App app;
	app.Run(startInfo);



	return 0;
}