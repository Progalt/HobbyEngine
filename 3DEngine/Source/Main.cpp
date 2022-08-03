
#include "Core/Application.h"
#include "Renderer/RenderManager.h"
#include "Core/Log.h"
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <imgui.h>



#include "Resources/Model.h"
#include "Scene/Scene.h"

class App : public Application
{
public:

	void Start() override
	{
		renderManager = RenderManager::Create(&window);
		
		ResourceManager::GetInstance().SetRenderManager(renderManager);

		model = ResourceManager::GetInstance().NewModel();

		model->LoadFromFile("Resources/WorldTest.pmdl", renderManager);
		model->materials[0]->roughness = 0.7f;

		sphere = ResourceManager::GetInstance().NewModel();

		sphere->LoadFromFile("Resources/Sphere.pmdl", renderManager);
		sphere->materials[0]->roughness = 0.3f;

		proj = glm::perspective(glm::radians(60.0f), (float)window.GetWidth() / (float)window.GetHeight(), 0.01f, 1000.0f);
		viewPos = { 0.0f, 0.0f, 0.0f };

		renderManager->ImGuiDraw([&]() { ImGuiRender(); });

		renderManager->aaMethod = AntiAliasingMethod::None;


		viewPos = glm::vec3(0.0f, 0.0f, 3.0f);

		renderManager->time = 45.0f;

		Actor* worldTest = scene.NewActor("World Test");
		worldTest->AddComponent<MeshRenderer>()->model = model;

		worldTest->GetTransform().SetEuler({ 90.0f, 0.0f, 0.0f });

		PostProcessCreateInfo fogCreateInfo{};
		fogCreateInfo.computeShader = true;
		fogCreateInfo.passGlobalData = true;
		fogCreateInfo.inputs =
		{
			PostProcessInput::Colour, PostProcessInput::Depth
		};
		fogCreateInfo.uniformBufferSize = sizeof(fogData);
		fogCreateInfo.shaderByteCode = FileSystem::ReadBytes("Resources/Shaders/Fog.comp.spv");

		fogEffect = renderManager->CreatePostProcessEffect(fogCreateInfo);

		renderManager->AddPostProcessEffect(fogEffect);
	}

	void Update() override
	{

		static int32_t lastX = 0, lastY = 0;

		static float pitch = 0.0f;
		static float yaw = -90.0f;

		glm::ivec2 mousePos = input.GetMousePosition();

		int32_t offX = mousePos.x - lastX;
		int32_t offY = mousePos.y - lastY;

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 up = glm::normalize(glm::cross(right, direction));


		if (input.IsButtonPressed(MouseButton::Right))
		{
			pitch += (float)offY;
			yaw += (float)offX;

			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}

		float velocity = 4.0f;

		if (input.IsKeyPressed(KeyCode::W))
		{
			viewPos += direction * velocity * time.delta;
		}

		if (input.IsKeyPressed(KeyCode::S))
		{
			viewPos -= direction * velocity * time.delta;
		}

		if (input.IsKeyPressed(KeyCode::A))
		{
			viewPos -= right * velocity * time.delta;
		}

		if (input.IsKeyPressed(KeyCode::D))
		{
			viewPos += right * velocity * time.delta;
		}


		view = glm::lookAt(viewPos, viewPos + direction, up);

		lastX = mousePos.x;
		lastY = mousePos.y;

		
	}


	glm::mat4 view;
	glm::mat4 proj;
	glm::vec3 viewPos;

	int framerate;


	void ImGuiRender()
	{
		ImGui::Begin("Statistics");

		framerate = GetFramerate((int)(1.0f / time.delta));

		ImGui::Text("FPS: %d", framerate);
		ImGui::Separator();
		ImGui::Text("Draw Calls: %d", renderManager->stats.drawCalls);
		ImGui::Text("Dispatch Calls: %d", renderManager->stats.dispatchCalls);
		ImGui::Text("Culled Meshes: %d", renderManager->stats.culledMeshes);
	
		ImGui::Separator();

		ImGui::Text("Settings");

		ImGui::DragFloat("Time", &renderManager->time, 1.0f, -180.0f, 180.0f);
		
		ImGui::Checkbox("Fog", &fogEffect->enabled);

		const char* tonemappingModes[] = { "None", "Filmic", "Unreal", "Uncharted 2", "ACES"};
		static const char* currentTonemap = "None";

		ImGui::Text("Tonemapping Mode");
		if (ImGui::BeginCombo("##combo", currentTonemap))
		{
			for (int n = 0; n < IM_ARRAYSIZE(tonemappingModes); n++)
			{
				bool is_selected = (currentTonemap == tonemappingModes[n]); 
				if (ImGui::Selectable(tonemappingModes[n], is_selected))
				{
					currentTonemap = tonemappingModes[n];
					if (currentTonemap == "None")
						renderManager->tonemappingMode = 0;
					else if (currentTonemap == "Filmic")
						renderManager->tonemappingMode = 1;
					else if (currentTonemap == "Unreal")
						renderManager->tonemappingMode = 2;
					else if (currentTonemap == "Uncharted 2")
						renderManager->tonemappingMode = 3;
					else if (currentTonemap == "ACES")
						renderManager->tonemappingMode = 4;
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::End();

	}

	SceneInfo info;

	void Render() override
	{

		info.hasDirectionalLight = 1;
		info.lightCount = 0;
		float t = -renderManager->time;
		info.dirLight.direction = { 0.0f, sin(glm::radians(t)), -cos(glm::radians(t)), 1.0f};
		info.dirLight.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
		
		fogData.sunDir = info.dirLight.direction;

		renderManager->UpdateScene(info);

		fogEffect->UpdateUniformBuffer(&fogData);

		scene.Render(renderManager);


		glm::mat4 viewProj = proj * view;

		CameraInfo cameraInfo{};
		cameraInfo.nearPlane = 0.001f;
		cameraInfo.farPlane = 1000.0f;
		cameraInfo.proj = proj;
		cameraInfo.view = view;
		cameraInfo.view_pos = viewPos;

		renderManager->Render(cameraInfo);

	}

	void Destroy() override
	{

		renderManager->WaitForIdle();

		fogEffect->Destroy();

		ResourceManager::GetInstance().Discard();

		RenderManager::Destroy(renderManager);
	}

	RenderManager* renderManager;

	struct
	{
		glm::vec4 sunDir;
	} fogData;
	PostProcessEffect* fogEffect;

	Scene scene;

	Model* model;
	Model* sphere;

	int GetFramerate(int newFrame)
	{
		// Average the framerate over 100 frames
		const int totalFrameAvg = 60;
		static int ptr = 0;
		static int lastFPS = 0;
		static std::array<int, totalFrameAvg> framerates;

		framerates[ptr] = newFrame;
		ptr++;

		// Only update the framerate every 100 frames
		if (ptr >= totalFrameAvg)
		{
			int total = 0;
			for (auto& i : framerates)
				total += i;

			lastFPS = (total / framerates.size());

			ptr = 0;
		}



		return lastFPS;
	}

	

};

int main(int argc, char* argv[])
{
	Application::StartInfo startInfo{};
	startInfo.width = 1280;
	startInfo.height = startInfo.width / 16 * 9;;
	startInfo.title = "3D Engine";

	App app;
	app.Run(startInfo);



	return 0;
}