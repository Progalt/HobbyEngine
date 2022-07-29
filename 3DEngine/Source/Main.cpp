
#include "Core/Application.h"
#include "Renderer/RenderManager.h"
#include "Core/Log.h"
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <imgui.h>



#include "Resources/Model.h"

class App : public Application
{
public:

	void Start() override
	{
		renderManager = RenderManager::Create(&window);

		ResourceManager::GetInstance().SetRenderManager(renderManager);

		model.LoadFromFile("Resources/Sphere.pmdl", renderManager);

		proj = glm::perspective(glm::radians(60.0f), (float)window.GetWidth() / (float)window.GetHeight(), 0.01f, 1000.0f);
		viewPos = { 0.0f, 0.0f, 0.0f };

		renderManager->ImGuiDraw([&]() { ImGuiRender(); });

		renderManager->aaMethod = AntiAliasingMethod::FastApproximateAA;


		viewPos = glm::vec3(0.0f, 0.0f, 3.0f);
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
		ImGui::Text("Renderpasses: %d", renderManager->stats.renderpasses);
	

		ImGui::End();
	}

	void Render() override
	{
		for (uint32_t i = 0; i < 4; i++)
		{
			for (uint32_t j = 0; j < 4; j++)
			{
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), { (float)i * 3, (float)j * 3, 0.0f });
				model.Queue(renderManager, transform);
			}
		}

		glm::mat4 viewProj = proj * view;

		renderManager->Render(viewProj, viewPos);

	}

	void Destroy() override
	{

		renderManager->WaitForIdle();

		model.Discard();

		ResourceManager::GetInstance().Discard();

		RenderManager::Destroy(renderManager);
	}

	RenderManager* renderManager;

	Model model;

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