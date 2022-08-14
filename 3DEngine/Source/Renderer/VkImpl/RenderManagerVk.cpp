
#include "RenderManagerVk.h"

#include "MeshVk.h"
#include "../../FileSystem/FileSystem.h"
#include "TextureVk.h"
#include "MaterialVk.h"
#include "../../Maths/Frustum.h"
#include "../../Core/Log.h"
#include <glm/gtc/matrix_transform.hpp>
#include "../../Threading/JobSystem.h"


RenderManagerVk::RenderManagerVk(Window* window, const RenderManagerCreateInfo& info)
{
	mProperties.renderWidth = info.renderWidth;
	mProperties.renderHeight = info.renderHeight;

	vk::DeviceCreateInfo createInfo{};

	createInfo.window = window;
	createInfo.threadCount = 1;
	createInfo.width = window->GetWidth();
	createInfo.height = window->GetHeight();
#ifdef _DEBUG
	createInfo.debugInfo = true;
#else 
	createInfo.debugInfo = true;
#endif
	createInfo.requestSRGBBackBuffer = true;
	createInfo.requestVSync = false;

	this->mProperties.width = window->GetWidth();
	this->mProperties.height = window->GetHeight();

	mDevice.Create(&createInfo);

	{
		vk::SamplerSettings settings{};
		settings.modeU = vk::WrapMode::Repeat;
		settings.modeV = vk::WrapMode::Repeat;
		settings.minFilter = vk::Filter::Linear;
		settings.magFilter = vk::Filter::Linear;
		settings.anisotropy = true;
		settings.maxAnisotropy = 3.0f;

		mDefaultSampler = mDevice.NewSampler(&settings);

		settings.modeU = vk::WrapMode::ClampToEdge;
		settings.modeV = vk::WrapMode::ClampToEdge;
		settings.minFilter = vk::Filter::Nearest;
		settings.magFilter = vk::Filter::Nearest;
		settings.anisotropy = false;
		settings.maxAnisotropy = mDevice.GetDeviceInfo().properties.maxAnisotropy;

		mTargetSampler = mDevice.NewSampler(&settings);
	}

	{
		vk::SamplerSettings settings{};
		settings.modeU = vk::WrapMode::Repeat;
		settings.modeV = vk::WrapMode::Repeat;
		settings.minFilter = vk::Filter::Linear;
		settings.magFilter = vk::Filter::Linear;
		settings.anisotropy = false;
		settings.maxAnisotropy = 3.0f;
		settings.compare = true;
		settings.compareOp = vk::CompareOp::Less;

		mShadowData.sampler = mDevice.NewSampler(&settings);
	}

	{

		globalDataManager.Create(&mDevice, &mGlobalDataStruct);

		mSceneInfo.lightCount = 0;
		mSceneInfo.hasDirectionalLight = 1;

		mSceneInfo.dirLight.direction = { -0.2f, -1.0f, -0.3f, 1.0f };
		mSceneInfo.dirLight.colour = { 1.0f, 1.0f, 1.0f, 1.0f };

		mSceneDataBuffer = mDevice.NewBuffer();
		mSceneDataBuffer.Create(vk::BufferType::Dynamic, vk::BufferUsage::Storage, sizeof(SceneInfo), &mSceneInfo);

	}

	{
		vk::RenderpassCreateInfo rpCreateInfo{};
		rpCreateInfo.type = vk::RenderpassType::Swapchain;
		rpCreateInfo.clearColour = { 0.0f , 0.0f, 0.0f, 1.0f };

		mRenderpasses.swapchain = mDevice.NewRenderpass(&rpCreateInfo);
	}

	{
		mGeometryPass.colourTarget = mDevice.NewTexture();
		mGeometryPass.colourTarget.CreateRenderTarget(vk::FORMAT_R8G8B8A8_SRGB, mProperties.renderWidth, mProperties.renderHeight);


		mGeometryPass.normalTarget = mDevice.NewTexture();
		mGeometryPass.normalTarget.CreateRenderTarget(vk::FORMAT_R8G8B8A8_UNORM, mProperties.renderWidth, mProperties.renderHeight);


		mGeometryPass.velocityTarget = mDevice.NewTexture();
		mGeometryPass.velocityTarget.CreateRenderTarget(vk::FORMAT_R16G16_SFLOAT, mProperties.renderWidth, mProperties.renderHeight);


		mGeometryPass.depthTarget = mDevice.NewTexture();
		mGeometryPass.depthTarget.CreateRenderTarget(vk::FORMAT_D32_SFLOAT, mProperties.renderWidth, mProperties.renderHeight);

		mGeometryPass.correctedDepthTarget = mDevice.NewTexture();
		mGeometryPass.correctedDepthTarget.CreateRenderTarget(vk::FORMAT_D32_SFLOAT, mProperties.renderWidth, mProperties.renderHeight);

		mGeometryPass.emissiveTarget = mDevice.NewTexture();
		mGeometryPass.emissiveTarget.CreateRenderTarget(vk::FORMAT_R8G8B8A8_UNORM, mProperties.renderWidth, mProperties.renderHeight);

		vk::RenderpassCreateInfo rpCreateInfo{};
		rpCreateInfo.type = vk::RenderpassType::Offscreen;
		rpCreateInfo.colourAttachments = { &mGeometryPass.colourTarget ,&mGeometryPass.normalTarget, &mGeometryPass.velocityTarget, &mGeometryPass.emissiveTarget };
		rpCreateInfo.depthAttachment = &mGeometryPass.depthTarget;
		rpCreateInfo.clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		rpCreateInfo.depthClear = 1.0f;
		rpCreateInfo.extentWidth = mProperties.renderWidth;
		rpCreateInfo.extentHeight = mProperties.renderHeight;
		
		mRenderpasses.geometryPass = mDevice.NewRenderpass(&rpCreateInfo);

		rpCreateInfo.type = vk::RenderpassType::Offscreen;
		rpCreateInfo.colourAttachments = {};
		rpCreateInfo.depthAttachment = &mGeometryPass.correctedDepthTarget;
		rpCreateInfo.clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		rpCreateInfo.depthClear = 1.0f;
		rpCreateInfo.extentWidth = mProperties.renderWidth;
		rpCreateInfo.extentHeight = mProperties.renderHeight;

		mGeometryPass.depthCorrectPass = mDevice.NewRenderpass(&rpCreateInfo);

		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();
		vk::ShaderBlob fragmentBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/fullscreen.vert.spv"));
		fragmentBlob.CreateFromSource(vk::ShaderStage::Fragment, FileSystem::ReadBytes("Resources/Shaders/depthCorrector.frag.spv"));

		vk::VertexDescription vertexDesc{};

		mGeometryPass.layout = mDevice.NewLayout();
		mGeometryPass.layout.AddLayoutBinding({ 0, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });

		mGeometryPass.layout.Create();

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mGeometryPass.depthCorrectPass;
		pipelineInfo.layout = { &mGeometryPass.layout };
		pipelineInfo.pushConstantRanges = {
		};
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = nullptr;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };


		mGeometryPass.depthCorrect = mDevice.NewPipeline(&pipelineInfo);

		vertexBlob.Destroy();
		fragmentBlob.Destroy();

		mGeometryPass.descriptor = mDevice.NewDescriptor(&mGeometryPass.layout);
		mGeometryPass.descriptor.BindCombinedImageSampler(&mGeometryPass.depthTarget, &mTargetSampler, 0);
		mGeometryPass.descriptor.Update();
	}

	mCmdList = mDevice.NewCommandList(vk::CommandListType::Primary);

	{

	}

	vk::VertexDescription baseVertexDesc{};
	baseVertexDesc.bindingDescs =
	{
		{ 0, sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(glm::vec3)}
	};

	baseVertexDesc.attributeDescs =
	{
		{ 0, 0, vk::Format::FORMAT_R32G32B32_SFLOAT, 0 },
		{ 0, 1, vk::Format::FORMAT_R32G32_SFLOAT, sizeof(glm::vec3)},
		{ 0, 2, vk::Format::FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec2)},
		{ 0, 3, vk::Format::FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3)},
	};

	{
		// Create base pipeline

		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();
		vk::ShaderBlob fragmentBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/base.vert.spv"));
		fragmentBlob.CreateFromSource(vk::ShaderStage::Fragment, FileSystem::ReadBytes("Resources/Shaders/base.frag.spv"));

		

		mBasePipeline.materialLayout = mDevice.NewLayout();
		mBasePipeline.materialLayout.AddLayoutBinding({ 0, vk::ShaderInputType::UniformBuffer, 1, vk::ShaderStage::Fragment });
		mBasePipeline.materialLayout.AddLayoutBinding({ 1, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mBasePipeline.materialLayout.AddLayoutBinding({ 2, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mBasePipeline.materialLayout.AddLayoutBinding({ 3, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mBasePipeline.materialLayout.AddLayoutBinding({ 4, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mBasePipeline.materialLayout.Create();


		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mRenderpasses.geometryPass;
		pipelineInfo.layout = { &mBasePipeline.materialLayout, globalDataManager.GetLayout(vk::ShaderStage::Vertex) };
		pipelineInfo.pushConstantRanges = {};
		pipelineInfo.cullMode = vk::CullMode::Front;
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = &baseVertexDesc;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };
		pipelineInfo.blending = false;

		pipelineInfo.pushConstantRanges =
		{
			{ vk::ShaderStage::Vertex, 0, sizeof(glm::mat4) * 2 }
		};

		mBasePipeline.pipeline = mDevice.NewPipeline(&pipelineInfo);

		vertexBlob.Destroy();
		fragmentBlob.Destroy();
	}

	{
		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();
		vk::ShaderBlob fragmentBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/fullscreen.vert.spv"));
		fragmentBlob.CreateFromSource(vk::ShaderStage::Fragment, FileSystem::ReadBytes("Resources/Shaders/fullscreen.frag.spv"));

		vk::VertexDescription vertexDesc{};

		mFullscreenPipeline.layout = mDevice.NewLayout();
		mFullscreenPipeline.layout.AddLayoutBinding({ 0, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });

		mFullscreenPipeline.layout.Create();

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mRenderpasses.swapchain;
		pipelineInfo.layout = { &mFullscreenPipeline.layout };
		pipelineInfo.pushConstantRanges = {
		{ vk::ShaderStage::Fragment, 0, sizeof(int) }
		};
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = nullptr;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };


		mFullscreenPipeline.pipeline = mDevice.NewPipeline(&pipelineInfo);

		vertexBlob.Destroy();
		fragmentBlob.Destroy();

		

	}

	{
		mCurrentOutput[0] = mDevice.NewTexture();
		mCurrentOutput[0].CreateRenderTarget(vk::FORMAT_R16G16B16A16_SFLOAT, mProperties.renderWidth, mProperties.renderHeight, true, vk::ImageLayout::General);

		mCurrentOutput[1] = mDevice.NewTexture();
		mCurrentOutput[1].CreateRenderTarget(vk::FORMAT_R16G16B16A16_SFLOAT, mProperties.renderWidth, mProperties.renderHeight, true, vk::ImageLayout::General);

		mHistory = mDevice.NewTexture();
		mHistory.CreateRenderTarget(vk::FORMAT_R16G16B16A16_SFLOAT, mProperties.renderWidth, mProperties.renderHeight, true, vk::ImageLayout::ShaderReadOnlyOptimal);

	}

	{


		mLightingPipeline.output = mDevice.NewTexture();
		mLightingPipeline.output.CreateRenderTarget(vk::FORMAT_R16G16B16A16_SFLOAT, mProperties.renderWidth, mProperties.renderHeight, true, vk::ImageLayout::General);

		mLightingPipeline.layout = mDevice.NewLayout();
		//mLightingPipeline.layout.AddLayoutBinding({ 0, vk::ShaderInputType::UniformBuffer, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 1, vk::ShaderInputType::StorageBuffer, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 2, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 3, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 4, vk::ShaderInputType::StorageImage, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 5, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 6, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.Create();

		mLightingPipeline.shadowLayout = mDevice.NewLayout();
		mLightingPipeline.shadowLayout.AddLayoutBinding({ 0, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });
		mLightingPipeline.shadowLayout.Create();

		mLightingPipeline.envLayout = mDevice.NewLayout();
		mLightingPipeline.envLayout.AddLayoutBinding({ 0, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });
		mLightingPipeline.envLayout.Create();

		vk::ShaderBlob shaderBlob = mDevice.NewShaderBlob();
		shaderBlob.CreateFromSource(vk::ShaderStage::Compute, FileSystem::ReadBytes("Resources/Shaders/lighting.comp.spv"));
		
		vk::ComputePipelineCreateInfo createInfo{};
		createInfo.computeBlob = &shaderBlob;
		createInfo.layout = { &mLightingPipeline.layout, globalDataManager.GetLayout(vk::ShaderStage::Compute), &mLightingPipeline.shadowLayout, &mLightingPipeline.envLayout };
		createInfo.pushConstantRanges = {
			{ vk::ShaderStage::Compute, 0, sizeof(int) * 2 }
		};

		mLightingPipeline.pipeline = mDevice.NewComputePipeline(&createInfo);

		mLightingPipeline.descriptor = mDevice.NewDescriptor(&mLightingPipeline.layout);
		//mLightingPipeline.descriptor.BindBuffer(&mGlobalData, 0, sizeof(GlobalData), 0);
		mLightingPipeline.descriptor.BindStorageBuffer(&mSceneDataBuffer, 0, sizeof(SceneInfo), 1);
		mLightingPipeline.descriptor.BindCombinedImageSampler(&mGeometryPass.colourTarget, &mTargetSampler, 2);
		mLightingPipeline.descriptor.BindCombinedImageSampler(&mGeometryPass.normalTarget, &mTargetSampler, 3);
		mLightingPipeline.descriptor.BindStorageImage(&mLightingPipeline.output, 4);
		mLightingPipeline.descriptor.BindCombinedImageSampler(&mGeometryPass.depthTarget, &mTargetSampler, 5);
		mLightingPipeline.descriptor.BindCombinedImageSampler(&mGeometryPass.emissiveTarget, &mTargetSampler, 6);
		mLightingPipeline.descriptor.Update();

		

		shaderBlob.Destroy();
	}



	

	// Sky
	{
		vk::RenderpassCreateInfo rpInfo{};
		rpInfo.colourAttachments = { &mLightingPipeline.output };
		rpInfo.depthAttachment = &mGeometryPass.depthTarget;
		rpInfo.depthInitialLayout = { vk::ImageLayout::ShaderReadOnlyOptimal };
		rpInfo.loadAttachments = true;
		rpInfo.loadDepth = true;
		rpInfo.extentWidth = mProperties.renderWidth;
		rpInfo.extentHeight = mProperties.renderHeight;
		rpInfo.type = vk::RenderpassType::Offscreen;
		rpInfo.colourAttachmentInitialLayouts = { vk::ImageLayout::General };
		rpInfo.colourAttachmentFinalLayouts = { vk::ImageLayout::General };	

		mSkyPass.renderpass = mDevice.NewRenderpass(&rpInfo);


		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();
		vk::ShaderBlob fragmentBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/sky.vert.spv"));
		fragmentBlob.CreateFromSource(vk::ShaderStage::Fragment, FileSystem::ReadBytes("Resources/Shaders/sky.frag.spv"));

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mSkyPass.renderpass;
		pipelineInfo.layout = { globalDataManager.GetLayout(vk::ShaderStage::Vertex)};
		pipelineInfo.pushConstantRanges = {
			{ vk::ShaderStage::Vertex, 0, sizeof(float) }
		};
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = nullptr;
		pipelineInfo.depthTest = true;
		pipelineInfo.depthWrite = false;
		pipelineInfo.compareOp = vk::CompareOp::Less;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };

		mSkyPass.pipeline = mDevice.NewPipeline(&pipelineInfo);

		

		vertexBlob.Destroy();
		fragmentBlob.Destroy();
	}

	mShadowData.directionalShadowMap.Create(&mDevice, QualitySetting::Medium);
	mShadowData.createdCascadeShadowMap = true;

	// Shadow pipeline
	{
		mShadowData.descriptorLayout = mDevice.NewLayout();
		mShadowData.descriptorLayout.AddLayoutBinding({ 0, vk::ShaderInputType::UniformBuffer, 1, vk::ShaderStage::Vertex });
		mShadowData.descriptorLayout.Create();


		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/shadow.vert.spv"));

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mShadowData.directionalShadowMap.renderpass;
		pipelineInfo.layout = { &mShadowData.descriptorLayout };
		pipelineInfo.pushConstantRanges = {
			{ vk::ShaderStage::Vertex, 0, sizeof(glm::mat4) + sizeof(int)}
		};
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = &baseVertexDesc;
		pipelineInfo.depthTest = true;
		pipelineInfo.depthWrite = true;
		pipelineInfo.compareOp = vk::CompareOp::Less;
		pipelineInfo.shaders = { &vertexBlob };

		mShadowData.pipeline = mDevice.NewPipeline(&pipelineInfo);



		vertexBlob.Destroy();
	}

	{

		mFullscreenPipeline.descriptor[0] = mDevice.NewDescriptor(&mFullscreenPipeline.layout);
		mFullscreenPipeline.descriptor[0].BindCombinedImageSampler(&mCurrentOutput[0], &mTargetSampler, 0);
		mFullscreenPipeline.descriptor[0].Update();

		mFullscreenPipeline.descriptor[1] = mDevice.NewDescriptor(&mFullscreenPipeline.layout);
		mFullscreenPipeline.descriptor[1].BindCombinedImageSampler(&mCurrentOutput[1], &mTargetSampler, 0);
		mFullscreenPipeline.descriptor[1].Update();


	}

	{
		probe = NewLightProbe(512);
		probe->position = { 0.0f, 2.0f, 0.0f };

		mLightProbes.push_back((LightProbeVk*)probe);
	}

	{
		ssaoOutput = mDevice.NewTexture();
		ssaoOutput.CreateRenderTarget( vk::FORMAT_R8_UNORM, mProperties.renderWidth, mProperties.renderHeight, true, vk::ImageLayout::ShaderReadOnlyOptimal);

		mCacao.Create(&mDevice);
		
		mCacao.SetUp(mProperties.renderWidth, mProperties.renderHeight, &mGeometryPass.correctedDepthTarget, nullptr, &ssaoOutput);

		vk::RenderpassCreateInfo rpInfo{};
		rpInfo.colourAttachments = { &mCurrentOutput[0] };
		rpInfo.depthAttachment = nullptr;
		rpInfo.loadAttachments = true;
		rpInfo.loadDepth = true;
		rpInfo.extentWidth = mProperties.renderWidth;
		rpInfo.extentHeight = mProperties.renderHeight;
		rpInfo.type = vk::RenderpassType::Offscreen;
		rpInfo.colourAttachmentInitialLayouts = { vk::ImageLayout::Undefined };
		rpInfo.colourAttachmentFinalLayouts = { vk::ImageLayout::General };

		mApplyAOPass.renderpass = mDevice.NewRenderpass(&rpInfo);

		mApplyAOPass.layout = mDevice.NewLayout();
		mApplyAOPass.layout.AddLayoutBinding({ 0, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mApplyAOPass.layout.AddLayoutBinding({ 1, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mApplyAOPass.layout.Create();

		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();
		vk::ShaderBlob fragmentBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/fullscreen.vert.spv"));
		fragmentBlob.CreateFromSource(vk::ShaderStage::Fragment, FileSystem::ReadBytes("Resources/Shaders/PostProcess/ApplyAO.frag.spv"));

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mApplyAOPass.renderpass;
		pipelineInfo.layout = { &mApplyAOPass.layout };
		pipelineInfo.pushConstantRanges = {
		};
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = nullptr;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };

		mApplyAOPass.pipeline = mDevice.NewPipeline(&pipelineInfo);

		mApplyAOPass.descriptor = mDevice.NewDescriptor(&mApplyAOPass.layout);
		mApplyAOPass.descriptor.BindCombinedImageSampler(&mLightingPipeline.output, &mTargetSampler, 0);
		mApplyAOPass.descriptor.BindCombinedImageSampler(&ssaoOutput, &mTargetSampler, 1);
		mApplyAOPass.descriptor.Update();


		vertexBlob.Destroy();
		fragmentBlob.Destroy();
	}

	{
		// Move to per light system

		mShadowResolve.resolvedShadow = mDevice.NewTexture();
		mShadowResolve.resolvedShadow.CreateRenderTarget(vk::FORMAT_R8_UNORM, mProperties.renderWidth, mProperties.renderHeight);

		vk::RenderpassCreateInfo rpInfo{};
		rpInfo.colourAttachments = { &mShadowResolve.resolvedShadow };
		rpInfo.depthAttachment = nullptr;
		rpInfo.loadAttachments = false;
		rpInfo.loadDepth = false;
		rpInfo.extentWidth = mProperties.renderWidth;
		rpInfo.extentHeight = mProperties.renderHeight;
		rpInfo.type = vk::RenderpassType::Offscreen;
		mShadowResolve.renderpass = mDevice.NewRenderpass(&rpInfo);

		mShadowResolve.layout = mDevice.NewLayout();
		mShadowResolve.layout.AddLayoutBinding({ 0, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mShadowResolve.layout.AddLayoutBinding({ 1, vk::ShaderInputType::StorageBuffer, 1, vk::ShaderStage::Fragment });
		mShadowResolve.layout.AddLayoutBinding({ 2, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mShadowResolve.layout.AddLayoutBinding({ 3, vk::ShaderInputType::UniformBuffer, 1, vk::ShaderStage::Fragment });
		mShadowResolve.layout.Create();

		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();
		vk::ShaderBlob fragmentBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/fullscreen.vert.spv"));
		fragmentBlob.CreateFromSource(vk::ShaderStage::Fragment, FileSystem::ReadBytes("Resources/Shaders/shadowResolve.frag.spv"));

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mShadowResolve.renderpass;
		pipelineInfo.layout = { globalDataManager.GetLayout(vk::ShaderStage::Fragment), &mShadowResolve.layout};
		pipelineInfo.pushConstantRanges = {
		};
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = nullptr;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };

		mShadowResolve.pipeline = mDevice.NewPipeline(&pipelineInfo);

		mShadowResolve.descriptor = mDevice.NewDescriptor(&mShadowResolve.layout);

		vertexBlob.Destroy();
		fragmentBlob.Destroy();
	}

	vk::Imgui::Init(&mDevice, window, &mRenderpasses.swapchain);

}

RenderManagerVk::~RenderManagerVk()
{
	mDevice.WaitIdle();

	probe->Destroy();

	ssaoOutput.Destroy();
	mCacao.Destroy();

	mApplyAOPass.layout.Destroy();
	mApplyAOPass.pipeline.Destroy();
	mApplyAOPass.renderpass.Destroy();

	mSceneDataBuffer.Destroy();

	mTargetSampler.Destroy();

	mCurrentOutput[0].Destroy();
	mCurrentOutput[1].Destroy();
	mHistory.Destroy();

	globalDataManager.Destroy();

	if (mShadowData.createdCascadeShadowMap)
	{
		mShadowData.directionalShadowMap.Destroy();
	}

	mSkyPass.renderpass.Destroy();
	mSkyPass.pipeline.Destroy();
	
	mRenderpasses.swapchain.Destroy();

	mGeometryPass.colourTarget.Destroy();
	mGeometryPass.depthTarget.Destroy();
	mGeometryPass.normalTarget.Destroy();
	mGeometryPass.velocityTarget.Destroy();
	mGeometryPass.emissiveTarget.Destroy();
	mGeometryPass.correctedDepthTarget.Destroy();
	mGeometryPass.depthCorrect.Destroy();
	mGeometryPass.depthCorrectPass.Destroy();
	mGeometryPass.layout.Destroy();

	mShadowData.pipeline.Destroy();
	mShadowData.descriptorLayout.Destroy();

	mLightingPipeline.output.Destroy();
	mLightingPipeline.layout.Destroy();
	mLightingPipeline.pipeline.Destroy();
	mLightingPipeline.shadowLayout.Destroy();
	mLightingPipeline.envLayout.Destroy();

	mShadowResolve.layout.Destroy();
	mShadowResolve.pipeline.Destroy();
	mShadowResolve.resolvedShadow.Destroy();
	mShadowResolve.renderpass.Destroy();

	mRenderpasses.geometryPass.Destroy();
	
	mBasePipeline.pipeline.Destroy();
	mBasePipeline.materialLayout.Destroy();

	mFullscreenPipeline.pipeline.Destroy();
	mFullscreenPipeline.layout.Destroy();

	mDefaultSampler.Destroy();
	mShadowData.sampler.Destroy();

	vk::Imgui::Destroy();

	mDevice.Destroy();
}

void RenderManagerVk::WaitForIdle()
{
	mDevice.WaitIdle();
}

void RenderManagerVk::Render(CameraInfo& cameraInfo)
{

	// This is a mega function that handles drawing the whole scene
	// It doesn't care about the main engine apart from that everything submitted is submitted in the correct format

	stats.drawCalls = 0;
	stats.dispatchCalls = 0;
	stats.culledMeshes = 0;

	static bool firstFrame = true;
	static uint64_t frameCount = 0;
	static uint32_t currentFrame = 0;

	
	currentFrame = (currentFrame == 0) ? 1 : 0;

	

	auto Halton = [](uint32_t i, uint32_t b)
	{
		float f = 1.0f;
		float r = 0.0f;

		while (i > 0)
		{
			f /= static_cast<float>(b);
			r = r + f * static_cast<float>(i % b);
			i = static_cast<uint32_t>(floorf(static_cast<float>(i) / static_cast<float>(b)));
		}

		return r;
	};

	uint32_t jitterIndex = frameCount % 8;

	float haltonX = 2.0f * Halton(jitterIndex + 1, 2) - 1.0f;
	float haltonY = 2.0f * Halton(jitterIndex + 1, 3) - 1.0f;
	float jitterX = (haltonX / (float)mProperties.renderWidth);
	float jitterY = (haltonY / (float)mProperties.renderWidth);

	glm::mat4 jitteredMatrix = glm::translate(glm::mat4(1.0f), { jitterX, jitterY, 0.0f });

	if (jitterVertices)
		mGlobalDataStruct.jitteredVP = jitteredMatrix * cameraInfo.proj * cameraInfo.view;
	else
		mGlobalDataStruct.jitteredVP = cameraInfo.proj * cameraInfo.view;

	mGlobalDataStruct.VP = cameraInfo.proj * cameraInfo.view;
	mGlobalDataStruct.prevVP = mCachedVP;
	mGlobalDataStruct.viewPos = glm::vec4(cameraInfo.view_pos, 1.0f);
	mGlobalDataStruct.inverseProj = glm::inverse(cameraInfo.proj);
	mGlobalDataStruct.inverseView = glm::inverse(cameraInfo.view);
	mGlobalDataStruct.view = cameraInfo.view;
	mGlobalDataStruct.proj = cameraInfo.proj;

	globalDataManager.UpdateData(&mGlobalDataStruct);

	mDevice.NextFrame();

	mCmdList.Begin();

	// First lets render some shadows

	
	if (mSceneInfo.hasDirectionalLight)
	{
			mShadowData.directionalShadowMap.UpdateCascades(mSceneInfo.dirLight, cameraInfo.nearPlane,
				cameraInfo.farPlane, cameraInfo.standardProj, cameraInfo.view);

		RenderDirectionalShadowMap(mCmdList, &mShadowData.directionalShadowMap);
	}

	// Lets render light probes

	RenderInfo renderInfo{};

	mCmdList.BeginDebugUtilsLabel("Light Probes");
	for (auto& lightProbe : mLightProbes)
	{ 
		if (lightProbe->update == false)
			continue;

		lightProbe->UpdateData();

		for (uint32_t i = 0; i < 6; i++)
		{
			renderInfo.data = lightProbe->data[i];
			renderInfo.globalManager = lightProbe->globalDataManager[i];
			
			renderInfo.renderWidth = lightProbe->resolution;
			renderInfo.renderHeight = lightProbe->resolution;
			renderInfo.target = &lightProbe->cubemap;
			renderInfo.level = i;

			RenderScene(renderInfo, mCmdList, true, false, false);
		}

		lightProbe->GenerateIrradiance(mCmdList);

		lightProbe->update = false;
	}
	mCmdList.EndDebugUtilsLabel();
	// Begin the Geometry pass
	// If you know about how a deferred renderer works this functions the same as that mostly.
	// Currently it outputs: 
	//		Target 1 : rgb = colour, a = unused
	//		Target 2 : rg = normal, b = roughness, a = metallic
	//		Target 3 : rg = velocity
	//		Target 4 : rgb = emissive
	// Thats just for now of course the ba in the target 2 is probably going to be metallic and roughness.
	// With a normal only taking rg its quite obvious it is encoded. 
	// 
	// Probably going to also store a material index that allows material data to be accessed during the lighting pass. 
	// This works if I store all material data in a large buffer and index into it

	// This render system allows me to render and specify a target to render too. It will then do the standard rendering pipeline and render it all


	renderInfo.data = mGlobalDataStruct;
	renderInfo.globalManager = globalDataManager;

	renderInfo.renderWidth = mProperties.renderWidth;
	renderInfo.renderHeight = mProperties.renderHeight;
	renderInfo.target = nullptr;
	renderInfo.level = 0;

	// Render the scene
	RenderScene(renderInfo, mCmdList, false, true, true);

	// CACAO ----

	if (cacao)
	{

		mCmdList.BeginDebugUtilsLabel("FidelityFX CACAO");

		mCacao.Execute(&mCmdList, renderInfo.data.proj, renderInfo.data.view);

		stats.dispatchCalls += 19;

		mLightingPipeline.output.Transition(vk::ImageLayout::General, vk::ImageLayout::ShaderReadOnlyOptimal, mCmdList);

		mCmdList.BeginRenderpass(&mApplyAOPass.renderpass, false);

		mCmdList.BindPipeline(&mApplyAOPass.pipeline);

		mCmdList.BindDescriptors({ &mApplyAOPass.descriptor }, &mApplyAOPass.pipeline, 0);

		mCmdList.Draw(3, 1, 0, 0);

		stats.drawCalls++;

		mCmdList.EndRenderpass();

		mLightingPipeline.output.Transition(vk::ImageLayout::ShaderReadOnlyOptimal, vk::ImageLayout::General, mCmdList);

		mCmdList.EndDebugUtilsLabel();

	}
	else
	{

		mLightingPipeline.output.Transition(vk::ImageLayout::General, vk::ImageLayout::TransferSrcOptimal, mCmdList);

		mCurrentOutput[0].Transition(vk::ImageLayout::General, vk::ImageLayout::TransferDstOptimal, mCmdList);

		vk::ImageCopy imageCopy{};
		imageCopy.dstLayer = 0;
		imageCopy.srcLayer = 0;
		imageCopy.w = mProperties.renderWidth;
		imageCopy.h = mProperties.renderHeight;
		imageCopy.srcX = 0;
		imageCopy.srcY = 0;
		imageCopy.dstX = 0;
		imageCopy.dstY = 0;
		mCmdList.CopyImage(&mLightingPipeline.output, vk::ImageLayout::TransferSrcOptimal, &mCurrentOutput[0], vk::ImageLayout::TransferDstOptimal, &imageCopy);

		mLightingPipeline.output.Transition(vk::ImageLayout::TransferSrcOptimal, vk::ImageLayout::General, mCmdList);

		mCurrentOutput[0].Transition(vk::ImageLayout::TransferDstOptimal, vk::ImageLayout::General, mCmdList);
	}
	// Lets start post process effects

	uint32_t targetNum = 1;
	uint32_t prevTarget = 0;

	uint32_t postProcessNum = 0;

	mCmdList.BeginDebugUtilsLabel("Post Process");

	for (uint32_t i = 0; i < mPostProcessEffects.size(); i++)
	{
		// Loop through the stack and apply each effect 

		PostProcessEffectVk* effect = mPostProcessEffects[i];

		postProcessNum++;
		
		if (effect->computeShader)
		{
			mCmdList.BindPipeline(&effect->computePipeline);

			// If the descriptors need generating. 
			// Generate them
			if (updatePostProcessStack)
			{
				Log::Info("Renderer", "(Re)Created Post Process Descriptors");
				effect->GenerateDescriptor(&mCurrentOutput[targetNum], &mCurrentOutput[prevTarget]);
			}

			mCurrentOutput[prevTarget].Transition(vk::ImageLayout::General, vk::ImageLayout::ShaderReadOnlyOptimal, mCmdList);

			if (effect->createInfo.passGlobalData)
				mCmdList.BindDescriptors({ globalDataManager.GetDescriptor(vk::ShaderStage::Compute), &effect->descriptor }, &effect->computePipeline, 0);
			else 
				mCmdList.BindDescriptors({ &effect->descriptor }, & effect->computePipeline, 0);

			mCmdList.Dispatch((int)ceilf((float)mProperties.renderWidth / THREAD_GROUP_SIZE), (int)ceilf((float)mProperties.renderHeight / THREAD_GROUP_SIZE), 1);

			stats.dispatchCalls++;

			// TODO: This could probably be improved

			mCurrentOutput[prevTarget].Transition(vk::ImageLayout::ShaderReadOnlyOptimal, vk::ImageLayout::General, mCmdList);

			if (effect->createInfo.cacheHistory)
			{
				// Copy the current output to the history
				mHistory.Transition(vk::ImageLayout::ShaderReadOnlyOptimal, vk::ImageLayout::General, mCmdList);

				vk::ImageCopy copy{};

				copy.srcLayer = 0;
				copy.dstLayer = 0;
				copy.srcX = 0; copy.srcY = 0;
				copy.dstX = 0; copy.dstY = 0;
				copy.w = mProperties.renderWidth;
				copy.h = mProperties.renderHeight;

				mCmdList.CopyImage(&mCurrentOutput[targetNum], vk::ImageLayout::General, &mHistory, vk::ImageLayout::General, &copy);

				mHistory.Transition(vk::ImageLayout::General, vk::ImageLayout::ShaderReadOnlyOptimal, mCmdList);
			}

		}

		prevTarget = targetNum;
		targetNum = (targetNum == 0) ? 1 : 0;

	}

	updatePostProcessStack = false;

	mCurrentOutput[prevTarget].Transition(vk::ImageLayout::General, vk::ImageLayout::ShaderReadOnlyOptimal, mCmdList);

	mCmdList.EndDebugUtilsLabel();

	/*if (aaMethod == AntiAliasingMethod::TemporalAA)
	{

	}
	else if (aaMethod == AntiAliasingMethod::FastApproximateAA)
	{
		mCmdList.BeginDebugUtilsLabel("FXAA");
		mCmdList.BeginRenderpass(&mFXAA.renderpass, false);

		mCmdList.BindPipeline(&mFXAA.pipeline);
		mCmdList.BindDescriptors({ &mFXAA.descriptor }, &mFXAA.pipeline, 0);

		mCmdList.Draw(3, 1, 0, 0);

		stats.drawCalls++;

		mCmdList.EndRenderpass();
		mCmdList.EndDebugUtilsLabel();
	}*/

	// This final pass brings it all together and presents to screen

	mCmdList.BeginDebugUtilsLabel("Present");

	mCmdList.BeginRenderpass(&mRenderpasses.swapchain, false);


	mCmdList.SetViewport(0, 0, this->mProperties.width, this->mProperties.height);
	mCmdList.SetScissor(0, 0, this->mProperties.width, this->mProperties.height);

	mCmdList.BindPipeline(&mFullscreenPipeline.pipeline);

	mCmdList.PushConstants(&mFullscreenPipeline.pipeline, vk::ShaderStage::Fragment, sizeof(int), 0, &tonemappingMode);

	mCmdList.BindDescriptors({ &mFullscreenPipeline.descriptor[prevTarget]}, &mFullscreenPipeline.pipeline, 0);

	mCmdList.Draw(3, 1, 0, 0);
	
	stats.drawCalls++;

	vk::Imgui::NewFrame();

	if (imguiFunc) 
		imguiFunc();

	vk::Imgui::Render(&mCmdList);

	mCmdList.EndRenderpass();

	mCmdList.EndDebugUtilsLabel();

	mCurrentOutput[prevTarget].Transition(vk::ImageLayout::ShaderReadOnlyOptimal, vk::ImageLayout::General, mCmdList);
	
	mCmdList.End();

	mDevice.SubmitCommandListsAndPresent({ mCmdList });

	// Cached VP for next frame

	mCachedVP = cameraInfo.proj * cameraInfo.view;

	firstFrame = false;
	frameCount++;

	mDeferredDraws.clear();

}

void RenderManagerVk::AddPostProcessEffect(PostProcessEffect* effect)
{
	mPostProcessEffects.push_back((PostProcessEffectVk*)effect);
}

void RenderManagerVk::RemovePostProcessEffect(PostProcessEffect* effect)
{
	PostProcessEffectVk* ef = (PostProcessEffectVk*)effect;

	for (uint32_t i = 0; i < mPostProcessEffects.size(); i++)
		if (mPostProcessEffects[i] == ef)
			mPostProcessEffects.erase(mPostProcessEffects.begin() + i);
}

PostProcessEffect* RenderManagerVk::CreatePostProcessEffect(const PostProcessCreateInfo& createInfo)
{
	return new PostProcessEffectVk(this, createInfo);
}

void RenderManagerVk::RenderDrawCmd(DrawCmd& cmd)
{

}

void RenderManagerVk::RenderDirectionalShadowMap(vk::CommandList& cmdList, CascadeShadowMap* shadowMap)
{

	cmdList.BeginDebugUtilsLabel("Directional Light Shadow Pass");

	std::deque<DrawCmd> draws(mDeferredDraws.begin(), mDeferredDraws.end());

	cmdList.BindPipeline(&mShadowData.pipeline);

	// render to each cascade
	for (uint32_t i = 0; i < CascadeCount; i++)
	{
		// all of it is rendered in one renderpass but the viewport is shifted
		shadowMap->SetupForRendering(cmdList, i);

		Frustum frustum(shadowMap->data.matrices[i]);

		// Loop through and draw
		for (auto& cmd : draws)
		{
			MeshVk* mesh = (MeshVk*)cmd.mesh;
			MaterialVk* mat = (MaterialVk*)cmd.material;
			
			// This doesn't count as culled because it was never meant to be drawn in the first place here
			if (!mat->castShadows)
				continue;

			mesh->boundingBox.Transform(cmd.transform);
			if (!frustum.Test(mesh->boundingBox))
			{
				// Remove it from the list
				// and continuee
				stats.culledMeshes++;
				continue;
			}

			struct
			{
				glm::mat4 model;
				int index;
			} constants;

			constants.model = cmd.transform;
			constants.index = i;

			if (!shadowMap->createdDescriptor)
				shadowMap->CreateDescriptor(&mShadowData.descriptorLayout, &mDevice);

			cmdList.BindDescriptors({ &shadowMap->descriptor }, &mShadowData.pipeline, 0);

			cmdList.PushConstants(&mShadowData.pipeline, vk::ShaderStage::Vertex, sizeof(constants), 0, &constants);

			cmdList.BindVertexBuffer(&mesh->vertexBuffer, 0);
			cmdList.BindIndexBuffer(&mesh->indexBuffer);

			cmdList.DrawIndexed((cmd.indexCount == 0) ? cmd.mesh->indices.size() : cmd.indexCount, 1, cmd.firstIndex, cmd.vertexOffset, 0);

			stats.drawCalls++;
		}

	
	}
	shadowMap->FinishRendering(cmdList);

	cmdList.EndDebugUtilsLabel();
}

void RenderManagerVk::RenderScene(RenderInfo& renderInfo, vk::CommandList& cmdList, bool renderSkyOnly, bool useIBL, bool reverseDepth)
{

	// Create a frustum for the current view projection
	// Objects are culled against this later
	Frustum frustum(renderInfo.data.proj * renderInfo.data.view);

	std::deque<DrawCmd> draws(mDeferredDraws.begin(), mDeferredDraws.end());

	if (renderSkyOnly)
		draws.clear();

	// Lets do a sort first. This reduces overdraw by drawing front to back
	std::sort(draws.begin(), draws.end(), [renderInfo](DrawCmd& a, DrawCmd& b)
		{
			float distA = glm::distance(glm::vec3(renderInfo.data.viewPos), glm::vec3(a.transform[3]));
			float distB = glm::distance(glm::vec3(renderInfo.data.viewPos), glm::vec3(b.transform[3]));

			return distA < distB;
		});

	cmdList.BeginDebugUtilsLabel("Geometry Pass");

	cmdList.BeginRenderpass(&mRenderpasses.geometryPass, false, renderInfo.renderWidth, renderInfo.renderHeight);

	if (reverseDepth)
		cmdList.SetViewport(0, 0, renderInfo.renderWidth, renderInfo.renderHeight, 1, 0);
	else 
		cmdList.SetViewport(0, 0, renderInfo.renderWidth, renderInfo.renderHeight);
	cmdList.SetScissor(0, 0, renderInfo.renderWidth, renderInfo.renderHeight);

	cmdList.BindPipeline(&mBasePipeline.pipeline);

	cmdList.BindDescriptors({ renderInfo.globalManager.GetDescriptor(vk::ShaderStage::Vertex) }, &mBasePipeline.pipeline, 1);


	for (auto& cmd : draws)
	{


		MaterialVk* mat = (MaterialVk*)cmd.material;
		MeshVk* mesh = (MeshVk*)cmd.mesh;

		// Cull the mesh against the frustums
		// It skips it and removes it from the draw list

		// Transform the bounding box to the transform specified
		mesh->boundingBox.Transform(cmd.transform);
		if (!frustum.Test(mesh->boundingBox))
		{
			// Remove it from the list 
			// and continuee
			stats.culledMeshes++;
			draws.pop_front();
			continue;
		}

		// If the descriptor isn't created. We should create it now
		// A Just In Time sort of scenario
		// This works when there are plenty of material combinations that can't be created in advanced 
		// Performance does take a hit on first use though
		if (!mat->createdDescriptor)
			mat->CreateDescriptor(&mDevice, &mBasePipeline.materialLayout);
		else
			mat->RegenDescriptor();


		// TODO: Should this be moved into a uniform buffer
		// Or maybe mix both

		// This is exactly 128 bytes
		// Which is the minimum required by the vulkan spec so for now its fine.
		struct
		{
			glm::mat4 prevModel;
			glm::mat4 model;
		} data;


		data.prevModel = cmd.transform;
		data.model = cmd.transform;


		cmdList.PushConstants(&mBasePipeline.pipeline, vk::ShaderStage::Vertex, sizeof(data), 0, &data);

		cmdList.BindDescriptors({ &mat->descriptor }, &mBasePipeline.pipeline, 0);

		// NOTE: For future I want to move this into a large vertex buffer and large index buffer that 
		// All drawable meshes use
		// This means I can bind once and then offset accordingly in draw calls

		cmdList.BindVertexBuffer(&mesh->vertexBuffer, 0);
		cmdList.BindIndexBuffer(&mesh->indexBuffer);

		cmdList.DrawIndexed((cmd.indexCount == 0) ? cmd.mesh->indices.size() : cmd.indexCount, 1, cmd.firstIndex, cmd.vertexOffset, 0);

		stats.drawCalls++;

		draws.pop_front();
	}


	cmdList.EndRenderpass();

	cmdList.EndDebugUtilsLabel();

	// Resolve the cascade Shadow maps to a rt

	cmdList.BeginDebugUtilsLabel("Shadow Map Resolve");

	cmdList.BeginRenderpass(&mShadowResolve.renderpass, false, renderInfo.renderWidth, renderInfo.renderHeight);

	cmdList.SetViewport(0, 0, renderInfo.renderWidth, renderInfo.renderHeight);

	if (!mShadowResolve.createdDescriptor)
	{
		mShadowResolve.descriptor.BindCombinedImageSampler(&mShadowData.directionalShadowMap.atlas, &mShadowData.sampler, 0);
		mShadowResolve.descriptor.BindStorageBuffer(&mSceneDataBuffer, 0, sizeof(SceneInfo), 1);
		mShadowResolve.descriptor.BindCombinedImageSampler(&mGeometryPass.depthTarget, &mTargetSampler, 2);
		mShadowResolve.descriptor.BindBuffer(&mShadowData.directionalShadowMap.uniformBuffer, 0, sizeof(mShadowData.directionalShadowMap.data), 3);
		mShadowResolve.descriptor.Update();

		mShadowResolve.createdDescriptor = true;
	}

	cmdList.BindPipeline(&mShadowResolve.pipeline);

	cmdList.BindDescriptors({ globalDataManager.GetDescriptor(vk::ShaderStage::Fragment), &mShadowResolve.descriptor }, &mShadowResolve.pipeline, 0);

	cmdList.Draw(3, 1, 0, 0);

	cmdList.EndRenderpass();

	cmdList.EndDebugUtilsLabel();

	// ------------------------------------

	cmdList.BeginDebugUtilsLabel("Depth Remap Pass");

	cmdList.SetViewport(0, 0, renderInfo.renderWidth, renderInfo.renderHeight);

	// Before anymore rendering we need to unreverse the depth buffer for post processing effects like FidelityFX CACAO

	cmdList.BeginRenderpass(&mGeometryPass.depthCorrectPass, false);

	cmdList.BindPipeline(&mGeometryPass.depthCorrect);
	cmdList.BindDescriptors({ &mGeometryPass.descriptor }, &mGeometryPass.depthCorrect, 0);

	cmdList.Draw(3, 1, 0, 0);

	stats.drawCalls++;

	cmdList.EndRenderpass();

	cmdList.EndDebugUtilsLabel();



	// Render the sky to the target
	// The sky is rendered first
	// and then the lighting is done over the top using the GBuffer
	{
		cmdList.BeginDebugUtilsLabel("Sky Pass");

		cmdList.BeginRenderpass(&mSkyPass.renderpass, false, renderInfo.renderWidth, renderInfo.renderHeight);

		cmdList.BindPipeline(&mSkyPass.pipeline);
		cmdList.BindDescriptors({ renderInfo.globalManager.GetDescriptor(vk::ShaderStage::Vertex) }, &mSkyPass.pipeline, 0);

		float correctTime = this->time * 2.0f - 10.0f;
		cmdList.PushConstants(&mSkyPass.pipeline, vk::ShaderStage::Vertex, sizeof(float), 0, &correctTime);

		cmdList.Draw(6, 1, 0, 0);

		stats.drawCalls++;

		cmdList.EndRenderpass();

		cmdList.EndDebugUtilsLabel();
	}

	if (!renderSkyOnly)
	{

		if (!mLightingPipeline.createdShadowDescriptor)
		{
			mLightingPipeline.shadowDescriptor = mDevice.NewDescriptor(&mLightingPipeline.shadowLayout);
			mLightingPipeline.shadowDescriptor.BindCombinedImageSampler(&mShadowResolve.resolvedShadow, &mDefaultSampler, 0);
			mLightingPipeline.shadowDescriptor.Update();

			mLightingPipeline.createdShadowDescriptor = true;
		}

		cmdList.BeginDebugUtilsLabel("Lighting Pass");
		// this is the lighting pass for deferred lighting. 

		// It is done in a compute shader instead of a fullscreen quad

		cmdList.BindPipeline(&mLightingPipeline.pipeline);

		struct
		{
			int ibl;
			int reverseDepth;
		} constants;

		constants.ibl = (int)useIBL;
		constants.reverseDepth = (int)reverseDepth;

		cmdList.PushConstants(&mLightingPipeline.pipeline, vk::ShaderStage::Compute, sizeof(int) * 2, 0, &constants);

		cmdList.BindDescriptors({ &mLightingPipeline.descriptor, renderInfo.globalManager.GetDescriptor(vk::ShaderStage::Compute), &mLightingPipeline.shadowDescriptor, &((LightProbeVk*)probe)->lightingDescriptor }, & mLightingPipeline.pipeline, 0);
		cmdList.Dispatch((int)ceilf((float)mProperties.renderWidth / THREAD_GROUP_SIZE), (int)ceilf((float)mProperties.renderHeight / THREAD_GROUP_SIZE), 1);

		stats.dispatchCalls++;



		cmdList.EndDebugUtilsLabel();
	}

	if (renderInfo.target != nullptr)
	{
		// We want to copy to the target if it is not nullptr
		vk::ImageCopy copy{};
		copy.srcLayer = 0;
		copy.dstLayer = renderInfo.level;
		copy.srcX = 0;
		copy.srcY = 0;
		copy.dstX = 0;
		copy.dstY = 0;
		copy.w = renderInfo.renderWidth;
		copy.h = renderInfo.renderHeight;
		cmdList.CopyImage(&mLightingPipeline.output, vk::ImageLayout::General, renderInfo.target, vk::ImageLayout::General, &copy);
	}
}

void RenderManagerVk::UpdateScene(SceneInfo sceneInfo)
{
	
	


	mSceneInfo = sceneInfo;
	mSceneDataBuffer.SetData(sizeof(SceneInfo), &sceneInfo);


}

void RenderManagerVk::UpdateSettings()
{
	if (mShadowData.directionalShadowMap.currentQuality != currentSettings.shadowQuality)
	{
		mLightingPipeline.createdShadowDescriptor = false;
		mShadowData.directionalShadowMap.Create(&mDevice, currentSettings.shadowQuality);
	}
}

void RenderManagerVk::QueueMesh(Mesh* mesh, Material* material, glm::mat4 transform, uint32_t firstIndex, uint32_t indexCount, uint32_t vertexOffset)
{
	if (material->pass == Pass::DontCare || material->pass == Pass::Deferred)
		mDeferredDraws.push_back({ mesh, transform, material, firstIndex, indexCount, vertexOffset });
	else
		mForwardDraws.push_back({ mesh, transform, material, firstIndex, indexCount, vertexOffset });
}


Mesh* RenderManagerVk::NewMesh()
{
	return new MeshVk(&mDevice);
}

Texture* RenderManagerVk::NewTexture()
{
	TextureVk* tex = new TextureVk(&mDevice);

	tex->sampler = &mDefaultSampler;

	return tex;
}

Material* RenderManagerVk::NewMaterial()
{
	MaterialVk* mat = new MaterialVk(&mDevice, &mBasePipeline.materialLayout);
	
	return mat;
}

LightProbe* RenderManagerVk::NewLightProbe(uint32_t resolution)
{
	LightProbeVk* probe = new LightProbeVk();
	probe->Create(&mDevice, resolution, &mLightingPipeline.envLayout, &mDefaultSampler);

	return probe;
}