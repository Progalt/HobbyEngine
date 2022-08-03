#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "../Core/Window.h"

#include "SkyMaterial.h"
#include <functional>

#include "GraphicsStructs.h"
#include "PostProcessEffect.h"

struct RenderStatistics
{
	uint32_t drawCalls;
	uint32_t dispatchCalls;
	uint32_t culledMeshes;

};

enum class AntiAliasingMethod
{
	None,

	// Use data from previous frames and velocity vectors to smooth egdes
	// Can cause loss of detail due to smoothing. 
	TemporalAA,


	// RESEARCH:
	// Looks like another good AA solution that works based on egde detection
	// Good for smoothing egdes but not shader aliasing stuff like TAA. 
	// SMAA,

	FastApproximateAA
};

struct RenderSettings
{
	// Method to do anti aliasing
	AntiAliasingMethod AntiAliasing;

	QualitySetting shadowQuality = QualitySetting::Medium;
};

struct CameraInfo
{
	glm::mat4 proj, view;
	glm::vec3 view_pos;
	float nearPlane, farPlane;
};

// Handles the rendering of the engine
class RenderManager
{
public:

	static RenderManager* Create(Window* window);

	static void Destroy(RenderManager* rm);

	virtual void WaitForIdle() = 0;

	// TODO: This system could probably be improved
	
	virtual void QueueMesh(Mesh* mesh, Material* material, glm::mat4 transform = glm::mat4(1.0f), uint32_t firstIndex = 0, uint32_t indexCount = 0, uint32_t vertexOffset = 0) = 0;


	virtual Mesh* NewMesh() = 0;

	virtual Texture* NewTexture() = 0;

	virtual Material* NewMaterial() = 0;

	virtual void SetSkyMaterial(SkyMaterial* material) = 0;

	virtual void Render(CameraInfo& cameraInfo) = 0;

	virtual void UpdateSettings() = 0;

	virtual void UpdateScene(SceneInfo sceneInfo) = 0;

	virtual void AddPostProcessEffect(PostProcessEffect* effect) = 0;

	virtual PostProcessEffect* CreatePostProcessEffect(const PostProcessCreateInfo& createInfo) = 0;

	void ImGuiDraw(std::function<void()> imgui)
	{
		imguiFunc = imgui;
	}

	RenderStatistics stats;

	AntiAliasingMethod aaMethod;

	RenderSettings currentSettings;

	int tonemappingMode = 0;

	// time is the time to render at. 
	float time = 0.0f;

	bool hasDirLight = false;
	DirectionalLight directionalLight;
	std::vector<PointLight> pointLights;

protected:

	std::function<void()> imguiFunc;
	
};