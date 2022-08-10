#pragma once

#include "../PostProcessEffect.h"

#include "../Vulkan/Device.h"

class RenderManagerVk;

class PostProcessEffectVk : public PostProcessEffect
{
public:

	PostProcessEffectVk(RenderManagerVk* renderManager, const PostProcessCreateInfo& createInfo);

	void UpdateUniformBuffer(void* data) override;

	void Destroy() override;

	void GenerateDescriptor(vk::Texture* target, vk::Texture* currentColour);

	bool computeShader;

	vk::ComputePipeline computePipeline;

	vk::Renderpass renderpass;
	vk::Pipeline pipeline;

	vk::DescriptorLayout descriptorLayout;
	vk::Descriptor descriptor;

	vk::Buffer uniformBuffer;

private:

	RenderManagerVk* rm;
};