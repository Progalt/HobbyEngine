
#include "ResourceManager.h"

#include "../Renderer/RenderManager.h"
#include "../Renderer/Material.h"

void ResourceManager::Discard()
{
	for (auto& texture : mLoadedTextures)
	{
		texture->Discard();
		delete texture;
	}

	for (auto& mat : mMaterials)
	{
		mat->Discard();
		delete mat;
	}
}

Material* ResourceManager::NewMaterial()
{
	mMaterials.push_back(mRenderManager->NewMaterial());

	return mMaterials[mMaterials.size() - 1];
}

Handle<Texture> ResourceManager::NewTexture()
{
	mLoadedTextures.push_back(mRenderManager->NewTexture());

	return Handle<Texture>(mLoadedTextures.size() - 1, 1);
}

Texture* ResourceManager::GetTexturePtr(Handle<Texture> texture)
{
	if (texture.Valid())
	{
		return mLoadedTextures[texture.mID];
	}

	return GetTexturePtr(GetErrorTexture());
}

Handle<Texture> ResourceManager::GetWhiteTexture()
{
	if (!mWhiteTexture.created)
	{
		mWhiteTexture.tex = NewTexture();

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
		mBlackTexture.tex = NewTexture();

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
		mErrorTexture.tex = NewTexture();

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