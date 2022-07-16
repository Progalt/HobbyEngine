
#include "Core/Application.h"
#include "Renderer/RenderManager.h"

class App : public Application
{
public:

	void Start() override
	{
		renderManager = RenderManager::Create(&window);

		mesh = renderManager->NewMesh();

		mesh->positions = {
			{ 0.0f, -0.5f, 0.0f },
			{ 0.5f, 0.5f, 0.0f },
			{ -0.5f, 0.5f, 0.0f }
		};

		mesh->indices = {
			0, 1, 2
		};

		mesh->GenerateMesh();

		mesh->material = ResourceManager::GetInstance().GetMaterial("Base");
	}

	void Update() override
	{
	}

	void Render() override
	{
		renderManager->QueueMesh(mesh);

		renderManager->Render();

	}

	void Destroy() override
	{
		RenderManager::Destroy(renderManager);
	}

	RenderManager* renderManager;

	Mesh* mesh;

};

int main(int argc, char* argv[])
{
	Application::StartInfo startInfo{};
	startInfo.width = 1280;
	startInfo.height = 720;
	startInfo.title = "3D Engine";

	App app;
	app.Run(startInfo);



	return 0;
}