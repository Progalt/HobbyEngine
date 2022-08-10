
#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Log.h"

	Image::~Image()
	{
		Discard();
	}

	void Image::Discard()
	{
		for (auto mip : mMip)
			stbi_image_free(mip.pixels);

		Log::Info("Image", "Freed Image");

		mDiscarded = true;
	}

	void Image::LoadFromMemory(const std::vector<int8_t>& file)
	{
		this->ready.store(false);

		if (file.size() == 0)
			Log::Error("Image", "Image file specified has size 0\n");

		if (!mDiscarded)
			Discard();

		mMip.resize(1);

		int w, h, c;
		mMip[0].pixels = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(file.data()), file.size(), &w, &h, &c, STBI_rgb_alpha);

		if (!mMip[0].pixels)
		{
			Log::Error("Image", "Failed to load pixel data from memory\n");
			return;
		}

		mMip[0].width = w;
		mMip[0].height = h;
		mBytesPerPixel = 4;

		Log::Info("Image", "Loaded image into memory of size: %d KB", ((w * h * 4) / 1000));

		mDiscarded = false;

		this->ready.store(true);
	}

	void Image::LoadFromFile(const std::string& filepath)
	{
		LoadFromMemory(FileSystem::ReadBytes(filepath));
	}
