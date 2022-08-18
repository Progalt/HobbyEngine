
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
		
		sphere = ResourceManager::GetInstance().NewModel();
		sphere->LoadFromFile("Resources/Sphere.pmdl", renderManager, true);
		sphere->materials[0]->emissiveColour = { 1.0f, 0.0f, 0.0f, 1.0f };

		viewPos = { 0.0f, 0.0f, 0.0f };

		renderManager->ImGuiDraw([&]() { ImGuiRender(); });

		viewPos = glm::vec3(0.0f, 0.0f, 3.0f);

		renderManager->time = 100.0f;

		Actor* worldTest = scene.NewActor("World Test");
		worldTest->AddComponent<MeshRenderer>()->model = model;
		worldTest->GetTransform().SetEuler({ 180.0f, 0.0f, 0.0f });
		worldTest->GetTransform().SetScale({ 0.05f, 0.05f, 0.05f });

		Actor* SphereActor = scene.NewActor("Sphere");
		SphereActor->AddComponent<MeshRenderer>()->model = sphere;
		SphereActor->GetTransform().SetPosition({ 0.0f, -3.0f, 0.0f });
		SphereActor->AddComponent<PointLight>()->colour = { 1.0f, 0.0f, 0.0f };
		SphereActor->GetComponent<PointLight>()->radius = 25.0f;
		SphereActor->GetComponent<PointLight>()->intensity = 150.0f;

		mainCam = scene.NewActor("Main Camera");
		mainCam->AddComponent<PerspectiveCamera>()->ConstructProjection();


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
		fxaaCreateInfo.cacheHistory = false;
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
		taaCreateInfo.cacheHistory = true;
		taaCreateInfo.shaderByteCode = FileSystem::ReadBytes("Resources/Shaders/TAA.comp.spv");

		taaEffect = renderManager->CreatePostProcessEffect(taaCreateInfo);

		renderManager->jitterVertices = false;

		renderManager->AddPostProcessEffect(fogEffect);
		renderManager->AddPostProcessEffect(fxaaEffect);
		//renderManager->AddPostProcessEffect(taaEffect);
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


		static float newPitch = pitch;
		static float newYaw = yaw;

		if (input.IsButtonPressed(MouseButton::Right))
		{
			newPitch -= (float)offY;
			newYaw += (float)offX;

			if (newPitch > 89.0f)
				newPitch = 89.0f;
			if (newPitch < -89.0f)
				newPitch = -89.0f;

		}

		pitch = lerp(pitch, newPitch, 1.0f / 3.0f);
		yaw = lerp(yaw, newYaw, 1.0f / 3.0f);


		glm::quat rot = mainCam->GetTransform().GetRotation();
		glm::quat xRot = glm::angleAxis(glm::radians(yaw), glm::vec3( 0.0f, 1.0f, 0.0f ));
		glm::quat yRot = glm::angleAxis(glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));

		mainCam->GetTransform().SetRotation(yRot * xRot);

		lastX = mousePos.x;
		lastY = mousePos.y;

		glm::vec3 direction = mainCam->GetTransform().GetForward();
		glm::vec3 right = -mainCam->GetTransform().GetRight();

		float velocity = 15.0f;

		static glm::vec3 newViewpos = viewPos;

		if (input.IsKeyPressed(KeyCode::Q))
			velocity *= 2.0f;

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
		mainCam->GetTransform().SetPosition(viewPos);

		
	}


	Actor* mainCam;

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

		if (renderManager->stats.renderedTriangles > 1000000)
		{
			double triNum = (double)renderManager->stats.renderedTriangles / 1000000.0;
			ImGui::Text("Rendered Triangles: %.2f Million", triNum);
		}
		else
		{
			ImGui::Text("Rendered Triangles: %d", renderManager->stats.renderedTriangles);
		}


		ImGui::Separator();

		ImGui::Text("Settings");

		ImGui::DragFloat("Time", &renderManager->time, 1.0f, -180.0f, 180.0f);
		
		
		if (fogEffect->enabled)
			ImGui::DragFloat("Fog Density", &fogData.fogDensity, 0.0001f, 0.0f, 0.5f, "%.5f");

		ImGui::Checkbox("CACAO", &renderManager->cacao);

		const char* shadowQualityModes[] = { "Ultra Low", "Low", "Medium", "High"};
		static const char* currentShadow = "Medium";

		ImGui::Text("Shadow Quality");
		if (ImGui::BeginCombo("##combo_shadow", currentShadow))
		{
			for (int n = 0; n < IM_ARRAYSIZE(shadowQualityModes); n++)
			{
				bool is_selected = (currentShadow == shadowQualityModes[n]);
				if (ImGui::Selectable(shadowQualityModes[n], is_selected))
				{
					currentShadow = shadowQualityModes[n];

					if (currentShadow == "High")
						renderManager->currentSettings.shadowQuality = QualitySetting::High;
					else if (currentShadow == "Medium")
						renderManager->currentSettings.shadowQuality = QualitySetting::Medium;
					else if (currentShadow == "Low")
						renderManager->currentSettings.shadowQuality = QualitySetting::Low;
					else if (currentShadow == "Ultra Low")
						renderManager->currentSettings.shadowQuality = QualitySetting::UltraLow;

					renderManager->shouldUpdateSettings = true;
				}
				if (is_selected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
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
		info.dirLight.colour *= 5.0f;

		std::vector<Actor*> pointLightView = scene.View<PointLight>();

		info.lightCount = pointLightView.size();

		for (uint32_t i = 0; i < info.lightCount; i++)
		{
			PointLight* l = pointLightView[i]->GetComponent<PointLight>();
			info.pointLights[i].position = glm::vec4(pointLightView[i]->GetTransform().GetPosition(), l->radius);
			info.pointLights[i].colour = glm::vec4(l->colour, l->intensity);
		}
		
		fogData.sunDir = info.dirLight.direction;

		renderManager->UpdateScene(info);

		fogEffect->UpdateUniformBuffer(&fogData);



		filmGrainUniforms.time = ticks;
		filmGrainUniforms.strength = 0.01f;
			
		filmGrainEffect->UpdateUniformBuffer(&filmGrainUniforms);
		

		taaData.firstFrame = (firstFrame) ? 1 : 0;
		taaEffect->UpdateUniformBuffer(&taaData);

		scene.Render(renderManager);

		Actor* mainCamera = scene.FindActor("Main Camera");

		PerspectiveCamera* camera = mainCamera->GetComponent<PerspectiveCamera>();

		CameraInfo cameraInfo{};
		cameraInfo.nearPlane = camera->settings.nearPlane;
		cameraInfo.farPlane = camera->settings.farPlane;
		cameraInfo.proj = camera->projection_reversedDepth;
		cameraInfo.prevViewProj = prevViewProj;
		cameraInfo.view = mainCamera->GetTransform().ComputeMatrix(glm::mat4(1.0f));
		cameraInfo.standardProj = camera->projection;
		cameraInfo.view_pos = -mainCamera->GetTransform().GetPosition();

		renderManager->Render(cameraInfo);

		firstFrame = false;

		prevViewProj = cameraInfo.proj * cameraInfo.view;
	}

	glm::mat4 prevViewProj;

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
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	Application::StartInfo startInfo{};
	startInfo.width = 1280;
	startInfo.height = startInfo.width / 16 * 9;;
	startInfo.title = "3D Engine";

	App app;
	app.Run(startInfo);

	//_CrtDumpMemoryLeaks();

	return 0;
}