#pragma once

#include <cstdint>

#include "../FileSystem/FileSystem.h"



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
