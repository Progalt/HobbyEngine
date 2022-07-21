
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Log.h"

	Image::~Image()
	{
		stbi_image_free(mPixels);

		Log::Info("Image", "Freed Image");
	}

	void Image::LoadFromMemory(const std::vector<int8_t>& file)
	{
		if (file.size() == 0)
			Log::Error("Image", "Image file specified has size 0\n");

		int w, h, c;
		mPixels = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(file.data()), file.size(), &w, &h, &c, STBI_rgb_alpha);

		if (!mPixels)
		{
			Log::Error("Image", "Failed to load pixel data from memory\n");
			return;
		}

		mWidth = w;
		mHeight = h;
		mBytesPerPixel = 4;

		Log::Info("Image", "Loaded image into memory of size: %d KB", ((w * h * 4) / 1000));
	}

	void Image::LoadFromFile(const std::string& filepath)
	{
		LoadFromMemory(FileSystem::ReadBytes(filepath));
	}
