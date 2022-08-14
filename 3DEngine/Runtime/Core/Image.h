#pragma once

#include <cstdint>

#include "../FileSystem/FileSystem.h"

#include "../Maths/Rect.h"

#include "../Resources/Resource.h"

	class Image : public Resource
	{
	public:

		void Discard() override;

		~Image();

		void LoadFromMemory(const std::vector< int8_t>& file);

		void LoadFromFile(const std::string& filepath);

		const uint32_t GetWidth(uint32_t mipLevel = 0) const { return mMip[0].width; }
		const uint32_t GetHeight(uint32_t mipLevel = 0) const { return mMip[0].height; }
		const uint32_t GetBytesPerPixel() const { return mBytesPerPixel; }

		const uint8_t* GetPixels(uint32_t mipLevel = 0)const { return mMip[0].pixels; }


	private:

		bool mDiscarded = true;

		uint32_t mBytesPerPixel;

		struct Mip
		{
			uint32_t width, height;
			uint8_t* pixels;
		};

		std::vector<Mip> mMip;
	};

	enum class CubemapSide
	{
		Up,
		Down,
		Front,
		Back,
		Left,
		Right
	};

	enum class CubemapImageFormat
	{
		// All the images are saved in individual files
		Individual,
		
		// All sides are in the same file in linear line
		Array,

		// All the images are in the same file in a cube net
		Net
	};

	class CubemapImage
	{
	public:

		Image sides[6];

		
	private:

		 

	};
