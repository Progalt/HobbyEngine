
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
		model = ResourceManager::GetInstance().NewModel();
		model->LoadFromFile("Resources/Sponza/Sponza.pmdl", renderManager, true);
		
		standardProj = glm::perspective(glm::radians(90.0f), (float)window.GetWidth() / (float)window.GetHeight(), 0.1f, 1000.0f);
		proj = ReversedDepthPerspective(glm::radians(90.0f), (float)window.GetWidth() / (float)window.GetHeight(), 0.1f);
		

		viewPos = { 0.0f, 0.0f, 0.0f };

		renderManager->ImGuiDraw([&]() { ImGuiRender(); });

		viewPos = glm::vec3(0.0f, 0.0f, 3.0f);

		renderManager->time = 100.0f;

		Actor* worldTest = scene.NewActor("World Test");
		worldTest->AddComponent<MeshRenderer>()->model = model;

		worldTest->GetTransform().SetEuler({ 180.0f, 0.0f, 0.0f });
		worldTest->GetTransform().SetScale({ 0.05f, 0.05f, 0.05f });

		//worldTest->GetTransform().SetScale({ 10.0f, 10.0f, 10.0f });

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

		PostProcessCreateInfo fxaaCreateInfo{};
		fxaaCreateInfo.computeShader = true;
		fxaaCreateInfo.passGlobalData = false;
		fxaaCreateInfo.inputs =
		{
			PostProcessInput::Colour
		};
		fxaaCreateInfo.uniformBufferSize = 0;
		fxaaCreateInfo.cacheHistory = true;
		fxaaCreateInfo.shaderByteCode = FileSystem::ReadBytes("Resources/Shaders/FXAA.comp.spv");

		fxaaEffect = renderManager->CreatePostProcessEffect(fxaaCreateInfo);

		PostProcessCreateInfo caCreateInfo{};
		caCreateInfo.computeShader = true;
		caCreateInfo.passGlobalData = false;
		caCreateInfo.inputs =
		{
			PostProcessInput::Colour
		};
		caCreateInfo.uniformBufferSize = 0;
		caCreateInfo.shaderByteCode = FileSystem::ReadBytes("Resources/Shaders/ChromaticAberration.comp.spv");

		chromaticAberrationEffect = renderManager->CreatePostProcessEffect(caCreateInfo);

		PostProcessCreateInfo filmGrainCreateInfo{};
		filmGrainCreateInfo.computeShader = true;
		filmGrainCreateInfo.passGlobalData = false;
		filmGrainCreateInfo.inputs =
		{
			PostProcessInput::Colour
		};
		filmGrainCreateInfo.uniformBufferSize = sizeof(filmGrainUniforms);
		filmGrainCreateInfo.shaderByteCode = FileSystem::ReadBytes("Resources/Shaders/PostProcess/FilmGrain.comp.spv");

		filmGrainEffect = renderManager->CreatePostProcessEffect(filmGrainCreateInfo);

		PostProcessCreateInfo taaCreateInfo{};
		taaCreateInfo.computeShader = true;
		taaCreateInfo.passGlobalData = false;
		taaCreateInfo.inputs =
		{
			PostProcessInput::Colour, PostProcessInput::Velocity, PostProcessInput::History, PostProcessInput::Depth
		};
		taaCreateInfo.uniformBufferSize = sizeof(taaData);
		taaCreateInfo.cacheHistory = false;
		taaCreateInfo.shaderByteCode = FileSystem::ReadBytes("Resources/Shaders/TAA.comp.spv");

		taaEffect = renderManager->CreatePostProcessEffect(taaCreateInfo);

		renderManager->jitterVertices = false;

		renderManager->AddPostProcessEffect(fogEffect);
		//renderManager->AddPostProcessEffect(taaEffect);
		renderManager->AddPostProcessEffect(fxaaEffect);
		//renderManager->AddPostProcessEffect(chromaticAberrationEffect);
		//renderManager->AddPostProcessEffect(filmGrainEffect);
	}


	void Update() override
	{

		static int32_t lastX = 0, lastY = 0;

		static float pitch = 0.0f;
		static float yaw = -90.0f;

		glm::ivec2 mousePos = input.GetMousePosition();

		float offX = floorf((float)mousePos.x - (float)lastX);
		float offY = floorf((float)mousePos.y - (float)lastY);

		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0.0f, 1.0f, 0.0f)));
		glm::vec3 up = glm::normalize(glm::cross(right, direction));

		static float newPitch = pitch;
		static float newYaw = yaw;

		if (input.IsButtonPressed(MouseButton::Right))
		{
			newPitch += (float)offY;
			newYaw += (float)offX;

			if (newPitch > 89.0f)
				newPitch = 89.0f;
			if (newPitch < -89.0f)
				newPitch = -89.0f;

		}

		pitch = lerp(pitch, newPitch, 1.0f / 3.0f);
		yaw = lerp(yaw, newYaw, 1.0f / 3.0f);

		lastX = mousePos.x;
		lastY = mousePos.y;

		float velocity = 15.0f;

		static glm::vec3 newViewpos = viewPos;

		if (input.IsKeyPressed(KeyCode::W))
		{
			newViewpos += direction * velocity * time.delta;
		}

		if (input.IsKeyPressed(KeyCode::S))
		{
			newViewpos -= direction * velocity * time.delta;
		}

		if (input.IsKeyPressed(KeyCode::A))
		{
			newViewpos -= right * velocity * time.delta;
		}

		if (input.IsKeyPressed(KeyCode::D))
		{
			newViewpos += right * velocity * time.delta;
		}

		viewPos = lerp(viewPos, newViewpos, glm::vec3(1.0f / 3.0f));

		view = glm::lookAt(viewPos, viewPos + direction, up);

		
	}

	Transform camTransform;


	 glm::mat4 view;
	 glm::mat4 proj;
	 glm::mat4 standardProj;
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
		
		
		if (fogEffect->enabled)
			ImGui::DragFloat("Fog Density", &fogData.fogDensity, 0.0001f, 0.0f, 0.5f, "%.5f");

		ImGui::Checkbox("CACAO", &renderManager->cacao);

		if (ImGui::Button("Update Cascades"))
		{
			renderManager->updateCascade = true;
		}

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

	float ticks = 0.0f;
	bool firstFrame = true;

	void Render() override
	{
		ticks += 0.05f;
		info.hasDirectionalLight = 1;
		info.lightCount = 0;
		float t = -renderManager->time;
		info.dirLight.direction = { 0.0f, sin(glm::radians(t)), -cos(glm::radians(t)), 1.0f};
		info.dirLight.colour = { 1.0f, 1.0f, 0.95f, 1.0f };
		info.dirLight.colour *= 10.0f;
		
		fogData.sunDir = info.dirLight.direction;

		renderManager->UpdateScene(info);

		fogEffect->UpdateUniformBuffer(&fogData);

		if (firstFrame)
		{

			filmGrainUniforms.time = ticks;
			filmGrainUniforms.strength = 0.01f;
			
			filmGrainEffect->UpdateUniformBuffer(&filmGrainUniforms);
		}

		//taaData.firstFrame = (firstFrame) ? 1 : 0;
		//taaEffect->UpdateUniformBuffer(&taaData);

		scene.Render(renderManager);

		glm::mat4 viewProj = proj * view;

		CameraInfo cameraInfo{};
		cameraInfo.nearPlane = 0.1f;
		cameraInfo.farPlane = 1000.0f;
		cameraInfo.proj = proj;
		cameraInfo.view = view;
		cameraInfo.standardProj = standardProj;
		cameraInfo.view_pos = viewPos;

		renderManager->Render(cameraInfo);

		firstFrame = false;
	}

	void Destroy() override
	{

		renderManager->WaitForIdle();

		fogEffect->Destroy();
		fxaaEffect->Destroy();
		chromaticAberrationEffect->Destroy();
		filmGrainEffect->Destroy();
		taaEffect->Destroy();

		delete fogEffect;
		delete fxaaEffect;
		delete chromaticAberrationEffect;
		delete filmGrainEffect;
		delete taaEffect;

		ResourceManager::GetInstance().Discard();

		RenderManager::Destroy(renderManager);
	}

	RenderManager* renderManager;

	struct
	{
		glm::vec4 sunDir;
		float fogDensity = 0.002;
		float padding[3];
	} fogData;
	PostProcessEffect* fogEffect;
	PostProcessEffect* fxaaEffect;
	PostProcessEffect* chromaticAberrationEffect;

	struct
	{
		int firstFrame;
		float padding[3];
	} taaData;
	PostProcessEffect* taaEffect;
	
	struct
	{
		float time;
		float strength;
		float padding[2];
	} filmGrainUniforms;
	PostProcessEffect* filmGrainEffect;

	Scene scene;

	Model* model;

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