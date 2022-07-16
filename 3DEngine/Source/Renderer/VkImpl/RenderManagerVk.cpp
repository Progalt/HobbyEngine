
#include "RenderManagerVk.h"

#include "MeshVk.h"
#include "../../FileSystem/FileSystem.h"

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
		rpCreateInfo.clearColour = { 0.0f, 0.0f, 0.0f, 1.0f };

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

		vk::PipelineCreateInfo pipelineInfo{};
		pipelineInfo.renderpass = &mRenderpasses.swapchain;
		pipelineInfo.layout = {};
		pipelineInfo.pushConstantRanges = {};
		pipelineInfo.topologyType = vk::Topology::TriangleList;
		pipelineInfo.vertexDesc = &vertexDesc;
		pipelineInfo.shaders = { &vertexBlob, &fragmentBlob };

		mBasePipeline = mDevice.NewPipeline(&pipelineInfo);

		vertexBlob.Destroy();
		fragmentBlob.Destroy();
	}
}

RenderManagerVk::~RenderManagerVk()
{
	mDevice.WaitIdle();

	mRenderpasses.swapchain.Destroy();

	mBasePipeline.Destroy();

	mDevice.Destroy();
}

void RenderManagerVk::Render()
{
	mDevice.NextFrame();

	mCmdList.Begin();

	mCmdList.BeginRenderpass(&mRenderpasses.swapchain, false);

	mCmdList.SetViewport(0, 0, this->mProperties.width, this->mProperties.height);
	mCmdList.SetScissor(0, 0, this->mProperties.width, this->mProperties.height);

	mCmdList.BindPipeline(&mBasePipeline);

	for (auto& cmd : mDrawCmds)
	{
		mCmdList.BindVertexBuffer(&((MeshVk*)cmd.mesh)->vertexBuffer, 0);
		mCmdList.BindIndexBuffer(&((MeshVk*)cmd.mesh)->indexBuffer);

		mCmdList.DrawIndexed(cmd.mesh->indices.size(), 1, 0, 0, 0);
	}

	mCmdList.EndRenderpass();

	mCmdList.End();

	mDevice.SubmitCommandListsAndPresent({ mCmdList });

	mDrawCmds.clear();
}

void RenderManagerVk::QueueMesh(Mesh* mesh)
{
	mDrawCmds.push_back({ mesh });
}

Mesh* RenderManagerVk::NewMesh()
{
	return new MeshVk(&mDevice);
}