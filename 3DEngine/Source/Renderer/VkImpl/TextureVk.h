#pragma once

#include "../Texture.h"

#include "../Vulkan/Device.h"

class TextureVk : public Texture
{
public:

	TextureVk(vk::Device* device);

	void CreateFromImage(const Image& img, const bool generateMipMaps, const bool srgb) override;

	void Create(const CreateInfo& createInfo) override;

	vk::Texture texture;

	vk::Sampler* sampler;

	vk::Device* device;
};