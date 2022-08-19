#pragma once


#include "../RenderManager.h"

#include "../Vulkan/Device.h"

#include "../Vulkan/ImguiImpl.h"

#include "GlobalData.h"
#include "ShadowAtlas.h"
#include <deque>
#include <vector>
#include "PostProcessEffectVk.h"
#include "LightProbeVk.h"
#include "CACAOImpl.h"
#include "../DebugRenderer.h"

#define THREAD_GROUP_SIZE 32

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

	LightProbe* NewLightProbe(uint32_t resolution) override;

	void QueueMesh(Mesh* mesh, Material* material, glm::mat4 transform = glm::mat4(1.0f), uint32_t firstIndex = 0, uint32_t indexCount = 0, uint32_t vertexOffset = 0, uint32_t drawCallIndex = 0) override;

	struct RenderInfo
	{
		vk::Texture* target;
		uint32_t level;
		glm::mat4 standardProj;

		uint32_t renderWidth, renderHeight;

		GlobalData data;
		GlobalDataManager globalManager;
	};

	void RenderScene( RenderInfo& renderInfo, vk::CommandList& cmdList, bool renderSkyOnly, bool useIBL, bool reverseDepth);

	void UpdateSettings() override;

	void UpdateScene(SceneInfo sceneInfo) override;

	void AddPostProcessEffect(PostProcessEffect* effect) override;

	void RemovePostProcessEffect(PostProcessEffect* effect) override;

	PostProcessEffect* CreatePostProcessEffect(const PostProcessCreateInfo& createInfo) override;

	void RenderDirectionalShadowMap(vk::CommandList& cmdList, CascadeShadowMap* shadowMap, RenderInfo& renderInfo);

	struct DrawCmd
	{
		Mesh* mesh;
		glm::mat4 transform;
		Material* material;
		uint32_t firstIndex, indexCount, vertexOffset;
		bool culled;
		uint32_t drawCallIndex;
	};

	void RenderDrawCmd(DrawCmd& cmd);

	void GuassianBlur(vk::Texture* tex, vk::Descriptor descriptors[2], vk::CommandList& cmdList);

	// Post process

	std::vector<PostProcessEffectVk*> mPostProcessEffects;
	bool mGeneratePostProcessDescriptors = true;

	std::deque<DrawCmd> mDeferredDraws;
	std::deque<DrawCmd> mForwardDraws;

	std::deque<LightProbeVk*> mLightProbes;
	LightProbe* probe;

	vk::Device mDevice;

	vk::CommandList mCmdList;
	vk::CommandList mShadowList;

	vk::Sampler mDefaultSampler;
	vk::Sampler mTargetSampler;

	GlobalData mGlobalDataStruct;

	SceneInfo mSceneInfo;
	vk::Buffer mSceneDataBuffer;

	vk::Texture mCurrentOutput[2];
	vk::Texture mHistory;

	GlobalDataManager globalDataManager;

	vk::Texture ssaoOutput;
	CACAO mCacao;

	struct
	{
		vk::Buffer boundVertexBuffer;
		vk::Buffer boundIndexBuffer;
	} mState;

	struct
	{
		int bloomMips = 8;
		vk::Texture brightTexture[8];
		vk::Texture bloomOutput;

		vk::ComputePipeline brightPipeline;

		vk::DescriptorLayout layout;
		vk::Descriptor descriptor[8];

		vk::ComputePipeline upPipeline;

		vk::Descriptor updescriptor[8];

	

	} mBloom;

	PostProcessEffect* mApplyBloom;

	struct
	{

		vk::Texture pingpong;

		vk::ComputePipeline pipeline;
		vk::DescriptorLayout layout;

		struct PushConstants
		{
			float scale;
			float strength;
			int direction;
		} pushConstants;

	} mGuassianBlur;

	struct
	{

		vk::Renderpass renderpass;
		vk::Pipeline pipeline;
		vk::DescriptorLayout layout;
		vk::Descriptor descriptor;


	} mApplyAOPass;

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
		vk::Renderpass renderpass;
		vk::Pipeline pipeline;

		vk::DescriptorLayout layout;
		vk::Descriptor descriptor;

		vk::Texture resolvedShadow;

		bool createdDescriptor;

	} mShadowResolve;

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

		vk::DescriptorLayout envLayout;

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

		vk::Texture correctedDepthTarget;

		vk::Pipeline depthCorrect;
		vk::DescriptorLayout layout;
		vk::Descriptor descriptor;
		vk::Renderpass depthCorrectPass;

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

};