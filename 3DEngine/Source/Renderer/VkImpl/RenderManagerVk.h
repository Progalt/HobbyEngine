#pragma once


#include "../RenderManager.h"

#include "../Vulkan/Device.h"

#include "../Vulkan/ImguiImpl.h"

#include <deque>

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

	void QueueMesh(Mesh* mesh, Material* material, glm::mat4 transform = glm::mat4(1.0f), uint32_t firstIndex = 0, uint32_t indexCount = 0) override;

	void SetSkyMaterial(SkyMaterial* material) override;


	struct DrawCmd
	{
		Mesh* mesh;
		glm::mat4 transform;
		Material* material;
		uint32_t firstIndex, indexCount;
	};

	std::deque<DrawCmd> mDeferredDraws;
	std::deque<DrawCmd> mForwardDraws;

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

		vk::Pipeline pipeline;
		vk::DescriptorLayout layout;

		vk::Descriptor descriptor;

	} mFullscreenPipeline;
	
	struct {

		vk::Texture colourTarget;
		vk::Texture normalTarget;
		vk::Texture velocityTarget;

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
	} mProperties;

	struct Sky
	{

		Mesh* skydome;

		bool generated;

		SkyMaterial* skyMaterial;

	} sky;
};