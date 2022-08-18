#pragma once

#include "Mesh.h"
#include "Texture.h"
#include "../Core/Window.h"

#include <functional>

#include "GraphicsStructs.h"
#include "PostProcessEffect.h"

#include "LightProbe.h"

struct RenderStatistics
{
	uint32_t drawCalls;
	uint32_t dispatchCalls;
	uint32_t culledMeshes;
	uint32_t renderedTriangles;
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
	QualitySetting shadowQuality = QualitySetting::High;
};

struct CameraInfo
{
	glm::mat4 proj, view, standardProj, prevViewProj;
	glm::vec3 view_pos;
	float nearPlane, farPlane;
};

struct RenderManagerCreateInfo
{
	uint32_t renderWidth, renderHeight;
};

// Handles the rendering of the engine
class RenderManager
{
public:

	static RenderManager* Create(Window* window, const RenderManagerCreateInfo& createInfo);

	static void Destroy(RenderManager* rm);

	virtual void WaitForIdle() = 0;

	// TODO: This system could probably be improved
	
	virtual void QueueMesh(Mesh* mesh, Material* material, glm::mat4 transform = glm::mat4(1.0f), uint32_t firstIndex = 0, uint32_t indexCount = 0, uint32_t vertexOffset = 0) = 0;


	virtual Mesh* NewMesh() = 0;

	virtual Texture* NewTexture() = 0;

	virtual Material* NewMaterial() = 0;

	virtual LightProbe* NewLightProbe(uint32_t resolution) = 0;

	virtual void Render(CameraInfo& cameraInfo) = 0;

	virtual void UpdateSettings() = 0;

	virtual void UpdateScene(SceneInfo sceneInfo) = 0;

	virtual void AddPostProcessEffect(PostProcessEffect* effect) = 0;

	virtual void RemovePostProcessEffect(PostProcessEffect* effect) = 0;

	virtual PostProcessEffect* CreatePostProcessEffect(const PostProcessCreateInfo& createInfo) = 0;

	void ImGuiDraw(std::function<void()> imgui)
	{
		imguiFunc = imgui;
	}

	bool updateCascade = true;

	RenderStatistics stats;

	AntiAliasingMethod aaMethod;

	RenderSettings currentSettings;

	bool shouldUpdateSettings = false;

	int tonemappingMode = 0;

	bool updatePostProcessStack = true;

	bool cacao = true;

	bool jitterVertices = false;

	// time is the time to render at. 
	float time = 0.0f;

	bool hasDirLight = false;
	DirectionalLight directionalLight;
	std::vector<GPUPointLight> pointLights;

protected:

	std::function<void()> imguiFunc;
	
};