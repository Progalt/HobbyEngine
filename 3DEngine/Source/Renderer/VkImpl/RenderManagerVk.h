#pragma once


#include "../RenderManager.h"

#include "../Vulkan/Device.h"

#include "../Vulkan/ImguiImpl.h"

#include "GlobalData.h"
#include "ShadowAtlas.h"
#include <deque>
#include <vector>
#include "PostProcessEffectVk.h"

class RenderManagerVk : public RenderManager
{
public:

	RenderManagerVk(Window* window, const RenderManagerCreateInfo& info);

	~RenderManagerVk();

	void WaitForIdle() override;

	void Render(CameraInfo& cameraInfo) override;

	Mesh* NewMesh() override;

	Texture* NewTexture() override;

	Material* NewMaterial() override;

	void QueueMesh(Mesh* mesh, Material* material, glm::mat4 transform = glm::mat4(1.0f), uint32_t firstIndex = 0, uint32_t indexCount = 0, uint32_t vertexOffset = 0) override;

	void SetSkyMaterial(SkyMaterial* material) override;

	struct RenderInfo
	{
		vk::Texture* target;
		uint32_t level;

		uint32_t renderWidth, renderHeight;

		GlobalData data;
		GlobalDataManager globalManager;
	};

	void RenderScene( RenderInfo& renderInfo, vk::CommandList& cmdList);

	void UpdateSettings() override;

	void UpdateScene(SceneInfo sceneInfo) override;

	void AddPostProcessEffect(PostProcessEffect* effect) override;

	void RemovePostProcessEffect(PostProcessEffect* effect) override;

	PostProcessEffect* CreatePostProcessEffect(const PostProcessCreateInfo& createInfo) override;

	void RenderDirectionalShadowMap(vk::CommandList& cmdList, CascadeShadowMap* shadowMap);

	struct DrawCmd
	{
		Mesh* mesh;
		glm::mat4 transform;
		Material* material;
		uint32_t firstIndex, indexCount, vertexOffset;
		bool culled;
	};

	void RenderDrawCmd(DrawCmd& cmd);

	glm::mat4 mCachedVP = glm::mat4(1.0f);

	// Post process

	std::vector<PostProcessEffectVk*> mPostProcessEffects;
	bool mGeneratePostProcessDescriptors = true;

	std::deque<DrawCmd> mDeferredDraws;
	std::deque<DrawCmd> mForwardDraws;

	vk::Device mDevice;

	vk::CommandList mCmdList;

	vk::Sampler mDefaultSampler;
	vk::Sampler mTargetSampler;

	GlobalData mGlobalDataStruct;

	SceneInfo mSceneInfo;
	vk::Buffer mSceneDataBuffer;

	vk::Texture mCurrentOutput[2];
	vk::Texture mHistory;

	GlobalDataManager globalDataManager;

	struct
	{
		CascadeShadowMap directionalShadowMap;
		bool createdCascadeShadowMap = false;

		vk::Pipeline pipeline;
		vk::DescriptorLayout descriptorLayout;

		vk::Sampler sampler;

	} mShadowData;

	struct
	{
		vk::Pipeline pipeline;

		vk::DescriptorLayout materialLayout;

	} mBasePipeline;

	struct
	{

		vk::Pipeline pipeline;

		vk::Renderpass renderpass;

		vk::Buffer vertexBuffer;

	} mDebugPass;


	struct
	{

		vk::Pipeline pipeline;
		vk::DescriptorLayout layout;

		vk::Descriptor descriptor[2];

	} mFullscreenPipeline;

	struct
	{
		vk::ComputePipeline pipeline;

		vk::DescriptorLayout layout;
		vk::Descriptor descriptor;

		vk::DescriptorLayout shadowLayout;
		vk::Descriptor shadowDescriptor;

		bool createdShadowDescriptor = false;

		vk::Texture output;
	} mLightingPipeline;

	struct
	{
		vk::Pipeline pipeline;

		vk::Renderpass renderpass;

	} mSkyPass;

	struct
	{

		vk::ComputePipeline pipeline;
		vk::DescriptorLayout layout;
		vk::Descriptor descriptor;

	} mFogPass;
	
	struct {

		vk::Texture colourTarget;
		vk::Texture normalTarget;
		vk::Texture velocityTarget;
		vk::Texture emissiveTarget;

		vk::Texture depthTarget;

	} mGeometryPass;


	struct
	{
		// Outputs all the required data to a GBuffer
		// for a lighting pass later on
		vk::Renderpass geometryPass;


		// Renders objects that could not be done in the deferred stage
		vk::Renderpass forwardPass;

		vk::Renderpass swapchain;

	} mRenderpasses;

	struct
	{
		uint32_t width, height;
		uint32_t renderWidth, renderHeight;
	} mProperties;

	struct Sky
	{

		Mesh* skydome;

		bool generated;

		SkyMaterial* skyMaterial;

	} sky;
};