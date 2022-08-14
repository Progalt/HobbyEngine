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

		int hasNormal;

		int roughnessMetallicPacked;

		glm::vec4 emissive;
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
		TextureVk* normalTex = (TextureVk*)ResourceManager::GetInstance().GetTexturePtr(normalMap);
		TextureVk* roughnessTex = (TextureVk*)ResourceManager::GetInstance().GetTexturePtr(roughnessMap);
		TextureVk* metallicTex = (TextureVk*)ResourceManager::GetInstance().GetTexturePtr(metallicMap);

		paramsBuffer.Create(vk::BufferType::Dynamic, vk::BufferUsage::Uniform, sizeof(MaterialParams), nullptr);

		setUpdate = true;

		RegenDescriptor();

		descriptor.SetAutoUpdate(false);
		descriptor.BindBuffer(&paramsBuffer, 0, sizeof(MaterialParams), 0);
		descriptor.BindCombinedImageSampler(&tex->texture, tex->sampler, 1);
		descriptor.BindCombinedImageSampler(&normalTex->texture, normalTex->sampler, 2);
		descriptor.BindCombinedImageSampler(&roughnessTex->texture, roughnessTex->sampler, 3);
		descriptor.BindCombinedImageSampler(&metallicTex->texture, metallicTex->sampler, 4);
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
		params.hasNormal = (int)normalMap.Valid();
		params.emissive = emissiveColour;
		params.roughnessMetallicPacked = (int)roughnessMetallicShareTexture;

		paramsBuffer.SetData(sizeof(MaterialParams), &params);

		setUpdate = false;
	}

	bool createdDescriptor = false;

	vk::Buffer paramsBuffer;

	vk::Descriptor descriptor;

};