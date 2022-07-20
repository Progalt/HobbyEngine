
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

	Image::~Image()
	{
		stbi_image_free(mPixels);
	}

	void Image::LoadFromMemory(const std::vector<int8_t>& file)
	{
		if (file.size() == 0)
			printf("Image file specified has size 0\n");

		int w, h, c;
		mPixels = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(file.data()), file.size(), &w, &h, &c, STBI_rgb_alpha);

		if (!mPixels)
		{
			printf("Failed to load pixel data from memory\n");
			return;
		}

		mWidth = w;
		mHeight = h;
		mBytesPerPixel = 4;
	}

	void Image::LoadFromFile(const std::string& filepath)
	{
		LoadFromMemory(FileSystem::ReadBytes(filepath));
	}
