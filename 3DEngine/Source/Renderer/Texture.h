#pragma once

#include <cstdint>

#include "../Core/Image.h"

#include "Vulkan/Format.h"

class Texture
{
public:

	struct CreateInfo
	{
		void* pixels;
		uint32_t width, height;
		uint32_t layers;
		bool generateMipMaps;

		vk::Format format;

		bool renderTarget;
	};

	virtual void CreateFromImage(const Image& img, const bool generateMipMaps, const bool srgb) = 0;

	virtual void Create(const CreateInfo& createInfo) = 0;

	uint32_t width, height;

};