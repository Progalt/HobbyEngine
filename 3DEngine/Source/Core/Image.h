#pragma once

#include <cstdint>

#include "../FileSystem/FileSystem.h"

#include "../Maths/Rect.h"

	class Image
	{
	public:

		~Image();

		void LoadFromMemory(const std::vector< int8_t>& file);

		void LoadFromFile(const std::string& filepath);

		const uint32_t GetWidth() const { return mWidth; }
		const uint32_t GetHeight() const { return mHeight; }
		const uint32_t GetBytesPerPixel() const { return mBytesPerPixel; }

		const uint8_t* GetPixels()const { return mPixels; }


	private:

		uint32_t mWidth, mHeight, mBytesPerPixel;
		uint8_t* mPixels;
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
