
#include "RenderManagerVk.h"

#include "MeshVk.h"
#include "../../FileSystem/FileSystem.h"
#include "TextureVk.h"
#include "MaterialVk.h"

#include <glm/gtc/matrix_transform.hpp>


RenderManagerVk::RenderManagerVk(Window* window)
{
	vk::DeviceCreateInfo createInfo{};

	createInfo.window = window;
	createInfo.threadCount = 1;
	createInfo.width = window->GetWidth();
	createInfo.height = window->GetHeight();
#ifdef _DEBUG
	createInfo.debugInfo = true;
#else 
	createInfo.debugInfo = false;
#endif
	createInfo.requestSRGBBackBuffer = true;
	createInfo.requestVSync = true;

	this->mProperties.width = window->GetWidth();
	this->mProperties.height = window->GetHeight();

	mDevice.Create(&createInfo);

	{
		vk::SamplerSettings settings{};
		settings.modeU = vk::WrapMode::Repeat;
		settings.modeV = vk::WrapMode::Repeat;
		settings.minFilter = vk::Filter::Nearest;
		settings.magFilter = vk::Filter::Nearest;
		settings.anisotropy = true;
		settings.maxAnisotropy = 3.0f;

		mDefaultSampler = mDevice.NewSampler(&settings);
	}

	{
		mGlobalData = mDevice.NewBuffer();
		mGlobalData.Create(vk::BufferType::Dynamic, vk::BufferUsage::Uniform, sizeof(mGlobalDataStruct), &mGlobalDataStruct);

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
		mGeometryPass.colourTarget.CreateRenderTarget(vk::FORMAT_R8G8B8A8_SRGB, window->GetWidth(), window->GetHeight());


		mGeometryPass.normalTarget = mDevice.NewTexture();
		mGeometryPass.normalTarget.CreateRenderTarget(vk::FORMAT_R16G16B16A16_SFLOAT, window->GetWidth(), window->GetHeight());


		mGeometryPass.velocityTarget = mDevice.NewTexture();
		mGeometryPass.velocityTarget.CreateRenderTarget(vk::FORMAT_R16G16_SFLOAT, window->GetWidth(), window->GetHeight());


		mGeometryPass.depthTarget = mDevice.NewTexture();
		mGeometryPass.depthTarget.CreateRenderTarget(vk::FORMAT_D32_SFLOAT, window->GetWidth(), window->GetHeight());

		vk::RenderpassCreateInfo rpCreateInfo{};
		rpCreateInfo.type = vk::RenderpassType::Offscreen;
		rpCreateInfo.colourAttachments = { &mGeometryPass.colourTarget ,&mGeometryPass.normalTarget, &mGeometryPass.velocityTarget };
		rpCreateInfo.depthAttachment = &mGeometryPass.depthTarget;
		rpCreateInfo.clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		rpCreateInfo.depthClear = 1.0f;
		
		mRenderpasses.geometryPass = mDevice.NewRenderpass(&rpCreateInfo);
	}

	mCmdList = mDevice.NewCommandList(vk::CommandListType::Primary);

	{
		// Create base pipeline

		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();
		vk::ShaderBlob fragmentBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/base.vert.spv"));
		fragmentBlob.CreateFromSource(vk::ShaderStage::Fragment, FileSystem::ReadBytes("Resources/Shaders/base.frag.spv"));

		vk::VertexDescription vertexDesc{};
		vertexDesc.bindingDescs =
		{
			{ 0, sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) }
		};

		vertexDesc.attributeDescs =
		{
			{ 0, 0, vk::Format::FORMAT_R32G32B32_SFLOAT, 0 },
			{ 0, 1, vk::Format::FORMAT_R32G32_SFLOAT, sizeof(glm::vec3)},
			{ 0, 2, vk::Format::FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec2)},
		};

		mBasePipeline.materialLayout = mDevice.NewLayout();
		mBasePipeline.materialLayout.AddLayoutBinding({ 0, vk::ShaderInputType::UniformBuffer, 1, vk::ShaderStage::Fragment });
		mBasePipeline.materialLayout.AddLayoutBinding({ 1, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mBasePipeline.materialLayout.Create();

		mBasePipeline.dataLayout = mDevice.NewLayout();
		mBasePipeline.dataLayout.AddLayoutBinding({ 0, vk::ShaderInputType::UniformBuffer, 1, vk::ShaderStage::Vertex });
		mBasePipeline.dataLayout.Create();

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mRenderpasses.geometryPass;
		pipelineInfo.layout = { &mBasePipeline.materialLayout, &mBasePipeline.dataLayout };
		pipelineInfo.pushConstantRanges = {};
		pipelineInfo.cullMode = vk::CullMode::Front;
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = &vertexDesc;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };
		pipelineInfo.blending = false;

		pipelineInfo.pushConstantRanges =
		{
			{ vk::ShaderStage::Vertex, 0, sizeof(glm::mat4) * 2 }
		};

		mBasePipeline.pipeline = mDevice.NewPipeline(&pipelineInfo);

		vertexBlob.Destroy();
		fragmentBlob.Destroy();

		mBasePipeline.dataDescriptor = mDevice.NewDescriptor(&mBasePipeline.dataLayout);
		mBasePipeline.dataDescriptor.BindBuffer(&mGlobalData, 0, sizeof(GlobalData), 0);
		mBasePipeline.dataDescriptor.Update();
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
		mCurrentOutput = mDevice.NewTexture();
		mCurrentOutput.CreateRenderTarget(vk::FORMAT_R16G16B16A16_SFLOAT, window->GetWidth(), window->GetHeight(), true, vk::ImageLayout::General);

		mHistory = mDevice.NewTexture();
		mHistory.CreateRenderTarget(vk::FORMAT_R16G16B16A16_SFLOAT, window->GetWidth(), window->GetHeight(), true, vk::ImageLayout::General);
	}

	{


		mLightingPipeline.output = mDevice.NewTexture();
		mLightingPipeline.output.CreateRenderTarget(vk::FORMAT_R16G16B16A16_SFLOAT, window->GetWidth(), window->GetHeight(), true, vk::ImageLayout::General);

		mLightingPipeline.layout = mDevice.NewLayout();
		mLightingPipeline.layout.AddLayoutBinding({ 0, vk::ShaderInputType::UniformBuffer, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 1, vk::ShaderInputType::StorageBuffer, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 2, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 3, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 4, vk::ShaderInputType::StorageImage, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.AddLayoutBinding({ 5, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });
		mLightingPipeline.layout.Create();

		vk::ShaderBlob shaderBlob = mDevice.NewShaderBlob();
		shaderBlob.CreateFromSource(vk::ShaderStage::Compute, FileSystem::ReadBytes("Resources/Shaders/lighting.comp.spv"));
		
		vk::ComputePipelineCreateInfo createInfo{};
		createInfo.computeBlob = &shaderBlob;
		createInfo.layout = { &mLightingPipeline.layout };

		mLightingPipeline.pipeline = mDevice.NewComputePipeline(&createInfo);

		mLightingPipeline.descriptor = mDevice.NewDescriptor(&mLightingPipeline.layout);
		mLightingPipeline.descriptor.BindBuffer(&mGlobalData, 0, sizeof(GlobalData), 0);
		mLightingPipeline.descriptor.BindStorageBuffer(&mSceneDataBuffer, 0, sizeof(SceneInfo), 1);
		mLightingPipeline.descriptor.BindCombinedImageSampler(&mGeometryPass.colourTarget, &mDefaultSampler, 2);
		mLightingPipeline.descriptor.BindCombinedImageSampler(&mGeometryPass.normalTarget, &mDefaultSampler, 3);
		mLightingPipeline.descriptor.BindStorageImage(&mLightingPipeline.output, 4);
		mLightingPipeline.descriptor.BindCombinedImageSampler(&mGeometryPass.depthTarget, &mDefaultSampler, 5);
		mLightingPipeline.descriptor.Update();

		shaderBlob.Destroy();
	}



	// FXAA

	{
		mFXAA.layout = mDevice.NewLayout();
		mFXAA.layout.AddLayoutBinding({ 0, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Fragment });
		mFXAA.layout.Create();

		vk::RenderpassCreateInfo rpInfo{};
		rpInfo.colourAttachments = { &mCurrentOutput };
		rpInfo.type = vk::RenderpassType::Offscreen;
		rpInfo.clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };
		rpInfo.colourAttachmentInitialLayouts = { vk::ImageLayout::General };

		mFXAA.renderpass = mDevice.NewRenderpass(&rpInfo);

		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();
		vk::ShaderBlob fragmentBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/fullscreen.vert.spv"));
		fragmentBlob.CreateFromSource(vk::ShaderStage::Fragment, FileSystem::ReadBytes("Resources/Shaders/FXAA.frag.spv"));

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mFXAA.renderpass;
		pipelineInfo.layout = { &mFXAA.layout };
		pipelineInfo.pushConstantRanges = {};
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = nullptr;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };

		mFXAA.pipeline = mDevice.NewPipeline(&pipelineInfo);

		mFXAA.descriptor = mDevice.NewDescriptor(&mFXAA.layout);
		mFXAA.descriptor.BindCombinedImageSampler(&mLightingPipeline.output, &mDefaultSampler, 0);
		mFXAA.descriptor.Update();

		vertexBlob.Destroy();
		fragmentBlob.Destroy();
	}

	// Sky
	{
		vk::RenderpassCreateInfo rpInfo{};
		rpInfo.colourAttachments = { &mLightingPipeline.output };
		rpInfo.depthAttachment = &mGeometryPass.depthTarget;
		rpInfo.depthInitialLayout = { vk::ImageLayout::ShaderReadOnlyOptimal };
		rpInfo.loadAttachments = true;
		rpInfo.loadDepth = true;
		rpInfo.type = vk::RenderpassType::Offscreen;
		rpInfo.colourAttachmentInitialLayouts = { vk::ImageLayout::General };
		// NOTE: It transitions to a shader resource immediately after this so we could do that within the renderpass its self
		rpInfo.colourAttachmentFinalLayouts = { vk::ImageLayout::General };	

		mSkyPass.renderpass = mDevice.NewRenderpass(&rpInfo);

		mSkyPass.layout = mDevice.NewLayout();
		mSkyPass.layout.AddLayoutBinding({ 0, vk::ShaderInputType::UniformBuffer, 1, vk::ShaderStage::Vertex });
		mSkyPass.layout.Create();

		vk::ShaderBlob vertexBlob = mDevice.NewShaderBlob();
		vk::ShaderBlob fragmentBlob = mDevice.NewShaderBlob();

		vertexBlob.CreateFromSource(vk::ShaderStage::Vertex, FileSystem::ReadBytes("Resources/Shaders/sky.vert.spv"));
		fragmentBlob.CreateFromSource(vk::ShaderStage::Fragment, FileSystem::ReadBytes("Resources/Shaders/sky.frag.spv"));

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mSkyPass.renderpass;
		pipelineInfo.layout = { &mSkyPass.layout };
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

		mSkyPass.descriptor = mDevice.NewDescriptor(&mSkyPass.layout);
		mSkyPass.descriptor.BindBuffer(&mGlobalData, 0, sizeof(GlobalData), 0);
		mSkyPass.descriptor.Update();
	}

	{

		mFullscreenPipeline.descriptor = mDevice.NewDescriptor(&mFullscreenPipeline.layout);
		mFullscreenPipeline.descriptor.BindCombinedImageSampler(&mLightingPipeline.output, &mDefaultSampler, 0);
		mFullscreenPipeline.descriptor.Update();

		mFullscreenPipeline.fxaaDescriptor = mDevice.NewDescriptor(&mFullscreenPipeline.layout);
		mFullscreenPipeline.fxaaDescriptor.BindCombinedImageSampler(&mCurrentOutput, &mDefaultSampler, 0);
		mFullscreenPipeline.fxaaDescriptor.Update();


	}


	vk::Imgui::Init(&mDevice, window, &mRenderpasses.swapchain);

}

RenderManagerVk::~RenderManagerVk()
{
	mDevice.WaitIdle();

	mGlobalData.Destroy();
	mSceneDataBuffer.Destroy();

	mCurrentOutput.Destroy();
	mHistory.Destroy();

	mFXAA.layout.Destroy();
	mFXAA.pipeline.Destroy();
	mFXAA.renderpass.Destroy();

	mSkyPass.renderpass.Destroy();
	mSkyPass.layout.Destroy();
	mSkyPass.pipeline.Destroy();
	
	mRenderpasses.swapchain.Destroy();

	mGeometryPass.colourTarget.Destroy();
	mGeometryPass.depthTarget.Destroy();
	mGeometryPass.normalTarget.Destroy();
	mGeometryPass.velocityTarget.Destroy();

	mLightingPipeline.output.Destroy();
	mLightingPipeline.layout.Destroy();
	mLightingPipeline.pipeline.Destroy();

	mRenderpasses.geometryPass.Destroy();
	
	mBasePipeline.pipeline.Destroy();
	mBasePipeline.dataLayout.Destroy();
	mBasePipeline.materialLayout.Destroy();

	mFullscreenPipeline.pipeline.Destroy();
	mFullscreenPipeline.layout.Destroy();

	mDefaultSampler.Destroy();

	vk::Imgui::Destroy();

	mDevice.Destroy();
}

void RenderManagerVk::WaitForIdle()
{
	mDevice.WaitIdle();
}

void RenderManagerVk::Render(const glm::mat4& view, const glm::vec3& view_pos, const glm::mat4& proj)
{

	// This is a mega function that handles drawing the whole scene
	// It doesn't care about the main engine apart from that everything submitted is submitted in the correct format

	stats.drawCalls = 0;
	stats.renderpasses = 0;

	static bool firstFrame = true;
	static uint64_t frameCount = 0;
	static uint32_t currentFrame = 0;

	
	currentFrame = (currentFrame == 0) ? 1 : 0;


	// Lets do a sort first. This reduces overdraw by drawing front to back
	std::sort(mDeferredDraws.begin(), mDeferredDraws.end(), [view_pos](DrawCmd& a, DrawCmd& b)
		{
			float distA = glm::distance(view_pos, glm::vec3(a.transform[3]));
			float distB = glm::distance(view_pos, glm::vec3(b.transform[3]));

			return distA < distB;
		});

	// Let's compute the jitter matrix
	// https://community.arm.com/arm-community-blogs/b/graphics-gaming-and-vr-blog/posts/temporal-anti-aliasing
	static const glm::vec2 SAMPLE_LOCS_16[16] = {
		glm::vec2(-8.0f, 0.0f) / 8.0f,
		glm::vec2(-6.0f, -4.0f) / 8.0f,
		glm::vec2(-3.0f, -2.0f) / 8.0f,
		glm::vec2(-2.0f, -6.0f) / 8.0f,
		glm::vec2(1.0f, -1.0f) / 8.0f,
		glm::vec2(2.0f, -5.0f) / 8.0f,
		glm::vec2(6.0f, -7.0f) / 8.0f,
		glm::vec2(5.0f, -3.0f) / 8.0f,
		glm::vec2(4.0f, 1.0f) / 8.0f,
		glm::vec2(7.0f, 4.0f) / 8.0f,
		glm::vec2(3.0f, 5.0f) / 8.0f,
		glm::vec2(0.0f, 7.0f) / 8.0f,
		glm::vec2(-1.0f, 3.0f) / 8.0f,
		glm::vec2(-4.0f, 6.0f) / 8.0f,
		glm::vec2(-7.0f, 8.0f) / 8.0f,
		glm::vec2(-5.0f, 2.0f) / 8.0f 
	};
	
	const unsigned SubsampleIdx = frameCount % 16;

	const glm::vec2 TexSize(1.0f / glm::vec2(mProperties.width, mProperties.height)); 
	const glm::vec2 SubsampleSize = TexSize * 2.0f;

	const glm::vec2 S = SAMPLE_LOCS_16[SubsampleIdx]; 

	glm::vec2 Subsample = S * SubsampleSize; 
	Subsample *= 0.5f;

	glm::mat4 jitteredMatrix = glm::translate(glm::mat4(1.0f), { Subsample.x, Subsample.y, 0.0f });

	if (aaMethod == AntiAliasingMethod::TemporalAA)
		mGlobalDataStruct.jitteredVP = proj * view * jitteredMatrix;
	else
		mGlobalDataStruct.jitteredVP = proj * view;

	mGlobalDataStruct.VP = proj * view;
	mGlobalDataStruct.prevVP = mCachedVP;
	mGlobalDataStruct.viewPos = glm::vec4(view_pos, 1.0f);
	mGlobalDataStruct.inverseProj = glm::inverse(proj);
	mGlobalDataStruct.inverseView = glm::inverse(view);
	mGlobalDataStruct.view = view;
	mGlobalDataStruct.proj = proj;

	mGlobalData.SetData(sizeof(GlobalData), &mGlobalDataStruct, 0);

	mDevice.NextFrame();

	mCmdList.Begin();
	
	// Begin the Geometry pass
	// If you know about how a deferred renderer works this functions the same as that mostly.
	// Currently it outputs: 
	//		Target 1 : rgb = colour, a = unused
	//		Target 2 : rg = normal, b = roughness, a = metallic
	//		Target 3 : rg = velocity
	// Thats just for now of course the ba in the target 2 is probably going to be metallic and roughness.
	// With a normal only taking rg its quite obvious it is encoded. 
	// 
	// Probably going to also store a material index that allows material data to be accessed during the lighting pass. 
	// This works if I store all material data in a large buffer and index into it

	mCmdList.BeginDebugUtilsLabel("Geometry Pass");

	mCmdList.BeginRenderpass(&mRenderpasses.geometryPass, false);

	stats.renderpasses++;

	mCmdList.SetViewport(0, 0, this->mProperties.width, this->mProperties.height);
	mCmdList.SetScissor(0, 0, this->mProperties.width, this->mProperties.height);

	mCmdList.BindPipeline(&mBasePipeline.pipeline);

	mCmdList.BindDescriptors({ &mBasePipeline.dataDescriptor }, &mBasePipeline.pipeline, 1);

	for (auto& cmd : mDeferredDraws)
	{

		MaterialVk* mat = (MaterialVk*)cmd.material;
		MeshVk* mesh = (MeshVk*)cmd.mesh;

		// If the descriptor isn't created. We should create it now
		// A Just In Time sort of scenario
		// This works when there are plenty of material combinations that can't be created in advanced 
		// Performance does take a hit on first use though
		if (!mat->createdDescriptor)
			mat->CreateDescriptor(&mDevice, &mBasePipeline.materialLayout);
		else
			mat->RegenDescriptor();


		// Pack all the data into a struct for now
		// So it can be done in one call
		// TODO: Should this be moved into a uniform buffer
		// Or maybe mix both
		
		// This is exactly 128 bytes
		// Which is the minimum required by vulkan so for now its fine.
		struct
		{
			glm::mat4 prevModel;
			glm::mat4 model;
		} data;

		
		data.prevModel = cmd.transform;
		data.model = cmd.transform;


		mCmdList.PushConstants(&mBasePipeline.pipeline, vk::ShaderStage::Vertex, sizeof(data), 0, &data);

		mCmdList.BindDescriptors({ &mat->descriptor }, &mBasePipeline.pipeline, 0);

		// NOTE: For future I want to move this into a large vertex buffer nad large index buffer that 
		// All drawable meshes use
		// This means I can bind once and then offset accordingly in draw calls

		mCmdList.BindVertexBuffer(&mesh->vertexBuffer, 0);
		mCmdList.BindIndexBuffer(&mesh->indexBuffer);

		mCmdList.DrawIndexed(( cmd.indexCount == 0) ? cmd.mesh->indices.size() : cmd.indexCount, 1, cmd.firstIndex, cmd.vertexOffset, 0);

		stats.drawCalls++;

		mDeferredDraws.pop_front();
	}

	mCmdList.EndRenderpass();

	mCmdList.EndDebugUtilsLabel();

	


	// Render the sky to the target
	{
		mCmdList.BeginDebugUtilsLabel("Sky Pass");

		mCmdList.BeginRenderpass(&mSkyPass.renderpass, false);

		mCmdList.BindPipeline(&mSkyPass.pipeline);
		mCmdList.BindDescriptors({ &mSkyPass.descriptor }, &mSkyPass.pipeline, 0);

		
		mCmdList.PushConstants(&mSkyPass.pipeline, vk::ShaderStage::Vertex, sizeof(float), 0, &this->time);

		mCmdList.Draw(6, 1, 0, 0);

		mCmdList.EndRenderpass();

		mCmdList.EndDebugUtilsLabel();
	}

	mCmdList.BeginDebugUtilsLabel("Lighting Pass");
	// this is the lighting pass for deferred lighting. 

	// It is done in a compute shader instead of a fullscreen quad

	mCmdList.BindPipeline(&mLightingPipeline.pipeline);

	mCmdList.BindDescriptors({ &mLightingPipeline.descriptor }, &mLightingPipeline.pipeline, 0);
	mCmdList.Dispatch(mProperties.width / 8, mProperties.height / 8, 1);


	vk::ImageBarrierInfo imgBarrier{};
	imgBarrier.srcAccess = vk::AccessFlags::ShaderWrite;
	imgBarrier.oldLayout = vk::ImageLayout::General;
	imgBarrier.dstAccess = vk::AccessFlags::ShaderRead;
	imgBarrier.newLayout = vk::ImageLayout::ShaderReadOnlyOptimal;

	mCmdList.ImageBarrier(&mLightingPipeline.output, vk::PipelineStage::ComputeShader, vk::PipelineStage::FragmentShader, imgBarrier);

	mCmdList.EndDebugUtilsLabel();

	if (aaMethod == AntiAliasingMethod::TemporalAA)
	{

	}

	if (aaMethod == AntiAliasingMethod::FastApproximateAA)
	{
		mCmdList.BeginDebugUtilsLabel("FXAA");
		mCmdList.BeginRenderpass(&mFXAA.renderpass, false);

		mCmdList.BindPipeline(&mFXAA.pipeline);
		mCmdList.BindDescriptors({ &mFXAA.descriptor }, &mFXAA.pipeline, 0);

		mCmdList.Draw(3, 1, 0, 0);

		mCmdList.EndRenderpass();
		mCmdList.EndDebugUtilsLabel();
	}

	

	// This final pass brings it all together and presents to screen

	mCmdList.BeginDebugUtilsLabel("Present");

	mCmdList.BeginRenderpass(&mRenderpasses.swapchain, false);




	stats.renderpasses++;

	mCmdList.SetViewport(0, 0, this->mProperties.width, this->mProperties.height);
	mCmdList.SetScissor(0, 0, this->mProperties.width, this->mProperties.height);

	mCmdList.BindPipeline(&mFullscreenPipeline.pipeline);

	mCmdList.PushConstants(&mFullscreenPipeline.pipeline, vk::ShaderStage::Fragment, sizeof(int), 0, &tonemappingMode);

	if (aaMethod == AntiAliasingMethod::None)
		mCmdList.BindDescriptors({ &mFullscreenPipeline.descriptor }, & mFullscreenPipeline.pipeline, 0);
	else if (aaMethod == AntiAliasingMethod::FastApproximateAA)
		mCmdList.BindDescriptors({ &mFullscreenPipeline.fxaaDescriptor }, &mFullscreenPipeline.pipeline, 0);
	mCmdList.Draw(3, 1, 0, 0);
	
	stats.drawCalls++;

	vk::Imgui::NewFrame();

	if (imguiFunc) 
		imguiFunc();

	vk::Imgui::Render(&mCmdList);

	mCmdList.EndRenderpass();

	mCmdList.EndDebugUtilsLabel();
	
	imgBarrier.srcAccess = vk::AccessFlags::ShaderRead;
	imgBarrier.oldLayout = vk::ImageLayout::ShaderReadOnlyOptimal;
	imgBarrier.dstAccess = vk::AccessFlags::ShaderWrite;
	imgBarrier.newLayout = vk::ImageLayout::General;

	mCmdList.ImageBarrier(&mLightingPipeline.output, vk::PipelineStage::FragmentShader, vk::PipelineStage::ComputeShader, imgBarrier);

	// This next part handles the copying of the current output to the history target
	// All the targets need to be transitioned to the correct layout and back again so it all works next frame
	{
		imgBarrier.srcAccess = vk::AccessFlags::ShaderRead;
		imgBarrier.oldLayout = vk::ImageLayout::ShaderReadOnlyOptimal;
		imgBarrier.dstAccess = vk::AccessFlags::TransferRead;
		imgBarrier.newLayout = vk::ImageLayout::TransferSrcOptimal;

		mCmdList.ImageBarrier(&mCurrentOutput, vk::PipelineStage::FragmentShader, vk::PipelineStage::Transfer, imgBarrier);

		imgBarrier.srcAccess = vk::AccessFlags::ShaderRead;
		imgBarrier.oldLayout = vk::ImageLayout::General;
		imgBarrier.dstAccess = vk::AccessFlags::TransferWrite;
		imgBarrier.newLayout = vk::ImageLayout::TransferDstOptimal;

		mCmdList.ImageBarrier(&mHistory, vk::PipelineStage::FragmentShader, vk::PipelineStage::Transfer, imgBarrier);

		vk::ImageCopy imgCopy{};
		imgCopy.srcX = 0, imgCopy.srcY = 0;
		imgCopy.dstX = 0, imgCopy.dstY = 0;
		imgCopy.w = mProperties.width;
		imgCopy.h = mProperties.height;

		mCmdList.CopyImage(&mCurrentOutput, vk::ImageLayout::TransferSrcOptimal, &mHistory, vk::ImageLayout::TransferDstOptimal, &imgCopy);

		imgBarrier.srcAccess = vk::AccessFlags::TransferRead;
		imgBarrier.oldLayout = vk::ImageLayout::TransferSrcOptimal;
		imgBarrier.dstAccess = vk::AccessFlags::ShaderWrite;
		imgBarrier.newLayout = vk::ImageLayout::General;

		mCmdList.ImageBarrier(&mCurrentOutput, vk::PipelineStage::Transfer, vk::PipelineStage::FragmentShader, imgBarrier);

		imgBarrier.srcAccess = vk::AccessFlags::TransferWrite;
		imgBarrier.oldLayout = vk::ImageLayout::TransferDstOptimal;
		imgBarrier.dstAccess = vk::AccessFlags::ShaderWrite;
		imgBarrier.newLayout = vk::ImageLayout::General;

		mCmdList.ImageBarrier(&mHistory, vk::PipelineStage::Transfer, vk::PipelineStage::ComputeShader, imgBarrier);
	}
	mCmdList.End();

	mDevice.SubmitCommandListsAndPresent({ mCmdList });

	// Cached VP for next frame

	mCachedVP = proj * view;

	firstFrame = false;
	frameCount++;
}

void RenderManagerVk::UpdateSettings()
{
	
}

void RenderManagerVk::QueueMesh(Mesh* mesh, Material* material, glm::mat4 transform, uint32_t firstIndex, uint32_t indexCount, uint32_t vertexOffset)
{
	if (material->pass == Pass::Deferred)
		mDeferredDraws.push_back({ mesh, transform, material, firstIndex, indexCount, vertexOffset });
	else
		mForwardDraws.push_back({ mesh, transform, material, firstIndex, indexCount, vertexOffset });
}


void RenderManagerVk::SetSkyMaterial(SkyMaterial* material)
{
	sky.skyMaterial = material;

	if (!sky.generated && material != nullptr)
	{
		// Generate the skydome
	}
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