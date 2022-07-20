
#include "RenderManagerVk.h"

#include "MeshVk.h"
#include "../../FileSystem/FileSystem.h"
#include "TextureVk.h"
#include "MaterialVk.h"

RenderManagerVk::RenderManagerVk(Window* window)
{
	vk::DeviceCreateInfo createInfo{};

	createInfo.window = window;
	createInfo.threadCount = 1;
	createInfo.width = window->GetWidth();
	createInfo.height = window->GetHeight();
	createInfo.debugInfo = true;
	createInfo.requestSRGBBackBuffer = false;

	this->mProperties.width = window->GetWidth();
	this->mProperties.height = window->GetHeight();

	mDevice.Create(&createInfo);

	{
		vk::RenderpassCreateInfo rpCreateInfo{};
		rpCreateInfo.type = vk::RenderpassType::Swapchain;
		rpCreateInfo.clearColour = { 0.3f, 0.2f, 0.6f, 1.0f };

		mRenderpasses.swapchain = mDevice.NewRenderpass(&rpCreateInfo);
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

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mRenderpasses.swapchain;
		pipelineInfo.layout = { &mBasePipeline.materialLayout };
		pipelineInfo.pushConstantRanges = {};
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = &vertexDesc;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };

		pipelineInfo.pushConstantRanges =
		{
			{ vk::ShaderStage::Vertex, 0, sizeof(glm::mat4) }
		};

		mBasePipeline.pipeline = mDevice.NewPipeline(&pipelineInfo);

		vertexBlob.Destroy();
		fragmentBlob.Destroy();
	}

	{
		vk::SamplerSettings settings{};
		settings.modeU = vk::WrapMode::Repeat;
		settings.modeV = vk::WrapMode::Repeat;
		settings.minFilter = vk::Filter::Nearest;
		settings.magFilter = vk::Filter::Nearest;
		
		mDefaultSampler = mDevice.NewSampler(&settings);
	}
}

RenderManagerVk::~RenderManagerVk()
{
	mDevice.WaitIdle();

	mRenderpasses.swapchain.Destroy();
	
	mBasePipeline.pipeline.Destroy();

	mBasePipeline.materialLayout.Destroy();

	mDefaultSampler.Destroy();

	mDevice.Destroy();
}

void RenderManagerVk::WaitForIdle()
{
	mDevice.WaitIdle();
}

void RenderManagerVk::Render(const glm::mat4& view_proj)
{
	mDevice.NextFrame();

	mCmdList.Begin();

	mCmdList.BeginRenderpass(&mRenderpasses.swapchain, false);

	mCmdList.SetViewport(0, 0, this->mProperties.width, this->mProperties.height);
	mCmdList.SetScissor(0, 0, this->mProperties.width, this->mProperties.height);

	mCmdList.BindPipeline(&mBasePipeline.pipeline);

	for (auto& cmd : mDrawCmds)
	{

		if (!((MaterialVk*)cmd.mesh->material)->createdDescriptor)
			((MaterialVk*)cmd.mesh->material)->CreateDescriptor(&mDevice, &mBasePipeline.materialLayout);
		else
			((MaterialVk*)cmd.mesh->material)->RegenDescriptor();

		glm::mat4 m = view_proj * cmd.transform;
		mCmdList.PushConstants(&mBasePipeline.pipeline, vk::ShaderStage::Vertex, sizeof(glm::mat4), 0, &m);

		mCmdList.BindDescriptors({ &((MaterialVk*)cmd.mesh->material)->descriptor }, &mBasePipeline.pipeline, 0);

		mCmdList.BindVertexBuffer(&((MeshVk*)cmd.mesh)->vertexBuffer, 0);
		mCmdList.BindIndexBuffer(&((MeshVk*)cmd.mesh)->indexBuffer);

		mCmdList.DrawIndexed(cmd.mesh->indices.size(), 1, 0, 0, 0);
	}

	mCmdList.EndRenderpass();

	mCmdList.End();

	mDevice.SubmitCommandListsAndPresent({ mCmdList });

	mDrawCmds.clear();
}

void RenderManagerVk::QueueMesh(Mesh* mesh, glm::mat4 transform)
{
	mDrawCmds.push_back({ mesh, transform });
}

void RenderManagerVk::QueueMesh(std::vector<Mesh*> mesh)
{
	for (auto& meshes : mesh)
		mDrawCmds.push_back({ meshes });
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