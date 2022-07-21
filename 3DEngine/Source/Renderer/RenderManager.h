#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "../Core/Window.h"

#include "SkyMaterial.h"
#include <functional>

struct RenderStatistics
{
	uint32_t drawCalls;
	uint32_t renderpasses;
};

// Handles the rendering of the engine
class RenderManager
{
public:

	static RenderManager* Create(Window* window);

	static void Destroy(RenderManager* rm);

	virtual void WaitForIdle() = 0;

	virtual void QueueMesh(Mesh* mesh, glm::mat4 transform = glm::mat4(1.0f)) = 0;

	virtual void QueueMesh(std::vector<Mesh*> mesh) = 0;

	virtual Mesh* NewMesh() = 0;

	virtual Texture* NewTexture() = 0;

	virtual Material* NewMaterial() = 0;

	virtual void SetSkyMaterial(SkyMaterial* material) = 0;

	virtual void Render(const glm::mat4& view_proj) = 0;

	void ImGuiDraw(std::function<void()> imgui)
	{
		imguiFunc = imgui;
	}

	RenderStatistics stats;

protected:

	std::function<void()> imguiFunc;
	
};