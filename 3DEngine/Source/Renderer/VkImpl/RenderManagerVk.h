#pragma once


#include "../RenderManager.h"

#include "../Vulkan/Device.h"

class RenderManagerVk : public RenderManager
{
public:

	RenderManagerVk(Window* window);

	~RenderManagerVk();

	void WaitForIdle() override;

	void Render(const glm::mat4& view_proj) override;

	Mesh* NewMesh() override;

	Texture* NewTexture() override;

	Material* NewMaterial() override;

	void QueueMesh(Mesh* mesh, glm::mat4 transform = glm::mat4(1.0f)) override;

	void QueueMesh(std::vector<Mesh*> mesh) override;

	void SetSkyMaterial(SkyMaterial* material) override;

	struct DrawCmd
	{
		Mesh* mesh;
		glm::mat4 transform;
	};

	std::vector<DrawCmd> mDrawCmds;

	vk::Device mDevice;

	vk::CommandList mCmdList;

	vk::Sampler mDefaultSampler;

	struct
	{
		vk::Pipeline pipeline;

		vk::DescriptorLayout materialLayout;

	} mBasePipeline;
	

	struct
	{

		vk::Renderpass swapchain;

	} mRenderpasses;

	struct
	{
		uint32_t width, height;
	} mProperties;

	struct Sky
	{

		Mesh* skydome;

		bool generated;

		SkyMaterial* skyMaterial;

	} sky;
};