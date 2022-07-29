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

		float roughness;
		float metallic;

		float padding[2];
	};

	void Discard() override
	{
		paramsBuffer.Destroy();
	}

	MaterialParams params;

	void CreateDescriptor(vk::Device* device, vk::DescriptorLayout* layout)
	{
		descriptor = device->NewDescriptor(layout);
		paramsBuffer = device->NewBuffer();

		TextureVk* tex = (TextureVk*)ResourceManager::GetInstance().GetTexturePtr(albedo);

		paramsBuffer.Create(vk::BufferType::Dynamic, vk::BufferUsage::Uniform, sizeof(MaterialParams), nullptr);

		setUpdate = true;

		RegenDescriptor();

		descriptor.SetAutoUpdate(false);
		descriptor.BindBuffer(&paramsBuffer, 0, sizeof(MaterialParams), 0);
		descriptor.BindCombinedImageSampler(&tex->texture, tex->sampler, 1);
		descriptor.Update();

		createdDescriptor = true;
	}

	void RegenDescriptor()
	{
		if (!setUpdate)
			return;

		params.albedo = albedoColour;
		params.roughness = roughness;
		params.metallic = metallic;

		paramsBuffer.SetData(sizeof(MaterialParams), &params);

		setUpdate = false;
	}

	bool createdDescriptor = false;

	vk::Buffer paramsBuffer;

	vk::Descriptor descriptor;

};