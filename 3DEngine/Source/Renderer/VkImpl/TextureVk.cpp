
#include "TextureVk.h"

TextureVk::TextureVk(vk::Device* device) : device(device)
{

}

void TextureVk::CreateFromImage(const Image& img, const bool generateMipMaps, const bool srgb)
{
	CreateInfo info;
	info.generateMipMaps = generateMipMaps;
	if (srgb)
	{
		if (img.GetBytesPerPixel() == 4)
			info.format = vk::FORMAT_R8G8B8A8_SRGB;
	}
	else
	{
		if (img.GetBytesPerPixel() == 4)
			info.format = vk::FORMAT_R8G8B8A8_UNORM;
	}

	info.width = img.GetWidth();
	info.height = img.GetHeight();
	info.renderTarget = false;
	info.layers = 1;
	info.pixels = (void*)img.GetPixels();

	Create(info);
}

void TextureVk::Create(const CreateInfo& createInfo)
{
	texture = device->NewTexture();
	

	if (createInfo.renderTarget)
	{
		texture.CreateRenderTarget(createInfo.format, createInfo.width, createInfo.height);
	}
	else
	{
		texture.Create(vk::TextureType::e2D, createInfo.format, createInfo.width, createInfo.height,
			createInfo.pixels, createInfo.layers,
			(createInfo.generateMipMaps) ? vk::CalculateMipLevels(createInfo.width, createInfo.height) : 1);
	}
}