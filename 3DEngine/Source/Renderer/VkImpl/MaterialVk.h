#pragma once

#include "../Material.h"
#include "TextureVk.h"
#include "../Vulkan/Device.h"

class MaterialVk : public Material
{
public:

	MaterialVk(vk::Device* device, vk::DescriptorLayout* layout)
	{
		//CreateDescriptor(device, layout);
	}

	struct MaterialParams
	{
		glm::vec4 albedo;
	};

	MaterialParams params;

	void CreateDescriptor(vk::Device* device, vk::DescriptorLayout* layout)
	{
		descriptor = device->NewDescriptor(layout);
		paramsBuffer = device->NewBuffer();

		TextureVk* tex = (TextureVk*)ResourceManager::GetInstance().GetTexturePtr(albedo);

		paramsBuffer.Create(vk::BufferType::Dynamic, vk::BufferUsage::Uniform, sizeof(MaterialParams), nullptr);

		RegenDescriptor();

		descriptor.SetAutoUpdate(false);
		descriptor.BindBuffer(&paramsBuffer, 0, sizeof(MaterialParams), 0);
		descriptor.BindCombinedImageSampler(&tex->texture, tex->sampler, 1);
		descriptor.Update();

		createdDescriptor = true;
	}

	void RegenDescriptor()
	{
		params.albedo = albedoColour;

		paramsBuffer.SetData(sizeof(MaterialParams), &params);
	}

	bool createdDescriptor = false;

	vk::Buffer paramsBuffer;

	vk::Descriptor descriptor;

};