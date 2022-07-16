#pragma once


#include "../RenderManager.h"

#include "../Vulkan/Device.h"

class RenderManagerVk : public RenderManager
{
public:

	RenderManagerVk(Window* window);

	~RenderManagerVk();

	void Render() override;

	Mesh* NewMesh() override;

	void QueueMesh(Mesh* mesh) override;


	struct DrawCmd
	{
		Mesh* mesh;
	};

	std::vector<DrawCmd> mDrawCmds;

	vk::Device mDevice;

	vk::CommandList mCmdList;

	vk::Pipeline mBasePipeline;

	struct
	{

		vk::Renderpass swapchain;

	} mRenderpasses;

	struct
	{
		uint32_t width, height;
	} mProperties;
};