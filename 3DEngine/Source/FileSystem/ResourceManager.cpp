
#include "ResourceManager.h"

#include "../Renderer/RenderManager.h"
#include "../Renderer/Material.h"

#include "../Core/Log.h"

#include "../Threading/JobSystem.h"

void ResourceManager::Discard()
{
	Log::Info("Resource Manager", "Freed %d texture(s)", mLoadedTextures.size());
	for (auto& texture : mLoadedTextures)
	{
		texture.texture->Discard();
		delete texture.texture;
	}

	Log::Info("Resource Manager", "Freed %d material(s)", mMaterials.size());

	for (auto& mat : mMaterials)
	{
		mat->Discard();
		delete mat;
	}

	Log::Info("Resource Manager", "Freed %d Model(s)", mModels.size());

	for (auto& model : mModels)
	{
		model->Discard();
		delete model;
	}
}

Material* ResourceManager::NewMaterial()
{
	mMaterials.push_back(mRenderManager->NewMaterial());

	return mMaterials[mMaterials.size() - 1];
}

Handle<Texture> ResourceManager::NewTexture(const std::string& name)
{
	uint32_t index = 0;
	for (auto& tex : mLoadedTextures)
	{
		if (tex.name == name)
			return Handle<Texture>(index, 1);

		index++;
	}

	mLoadedTextures.push_back({ name, mRenderManager->NewTexture() });

	return Handle<Texture>(mLoadedTextures.size() - 1, 1);
}

Model* ResourceManager::NewModel()
{
	mModels.push_back(new Model());

	return mModels[mModels.size() - 1];
}

Texture* ResourceManager::GetTexturePtr(Handle<Texture> texture)
{
	if (texture.Valid())
	{
		return mLoadedTextures[texture.mID].texture;
	}

	return GetTexturePtr(GetErrorTexture());
}

Handle<Texture> ResourceManager::GetWhiteTexture()
{
	if (!mWhiteTexture.created)
	{
		mWhiteTexture.tex = NewTexture("WhiteTex");

		char pixels[16] = { 255,255,255,255,  255,255,255,255, 255,255,255,255, 255,255,255,255 };

		Texture::CreateInfo createInfo{};
		createInfo.format = vk::Format::FORMAT_R8G8B8A8_SRGB;
		createInfo.generateMipMaps = false;
		createInfo.width = 2;
		createInfo.height = 2;
		createInfo.layers = 1;
		createInfo.renderTarget = false;
		createInfo.pixels = pixels;

		GetTexturePtr(mWhiteTexture.tex)->Create(createInfo);

		mWhiteTexture.created = true;
	}

	return mWhiteTexture.tex;
}

Handle<Texture> ResourceManager::GetBlackTexture()
{
	if (!mBlackTexture.created)
	{
		mBlackTexture.tex = NewTexture("BlackTex");

		char pixels[16] = { 0,0,0,255,  0,0,0,255, 0,0,0,255, 0,0,0,255 };

		Texture::CreateInfo createInfo{};
		createInfo.format = vk::Format::FORMAT_R8G8B8A8_SRGB;
		createInfo.generateMipMaps = false;
		createInfo.width = 2;
		createInfo.height = 2;
		createInfo.layers = 1;
		createInfo.renderTarget = false;
		createInfo.pixels = pixels;

		GetTexturePtr(mBlackTexture.tex)->Create(createInfo);

		mBlackTexture.created = true;
	}

	return mBlackTexture.tex;
}

Handle<Texture> ResourceManager::GetErrorTexture()
{
	if (!mErrorTexture.created)
	{
		mErrorTexture.tex = NewTexture("ErrorTex");

		char pixels[16] = { 0,0,0,255,255,0,255,255,  255,0,255,255,  0,0,0,255 };

		Texture::CreateInfo createInfo{};
		createInfo.format = vk::Format::FORMAT_R8G8B8A8_SRGB;
		createInfo.generateMipMaps = false;
		createInfo.width = 2;
		createInfo.height = 2;
		createInfo.layers = 1;
		createInfo.renderTarget = false;
		createInfo.pixels = pixels;

		GetTexturePtr(mErrorTexture.tex)->Create(createInfo);

		mErrorTexture.created = true;
	}

	return mErrorTexture.tex;

}

Image* ResourceManager::StreamFromDisk(const std::string& path)
{
	Image* img = new Image();
	
	JobSystem::Execute([img, path]()
		{
			img->LoadFromFile(path);
		});

	return img;
}