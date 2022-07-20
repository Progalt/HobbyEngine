#pragma once

#include <cstdint>

#include "../Core/Singleton.h"

#include "../Renderer/Texture.h"
#include <string>


#include <vector>

class RenderManager;
class Material;

template<typename _Ty>
class Handle
{
public:

	Handle() : mID(0), mPool(0) { }
	Handle(uint32_t id, uint32_t pool): mID(id), mPool(pool) { }

	bool Valid() { return (mPool != 0); }

private:

	friend class ResourceManager;

	uint32_t mID, mPool;

};


class ResourceManager
{
public:

	DECLARE_SINGLETON(ResourceManager)

	void SetRenderManager(RenderManager* rm)
	{
		this->mRenderManager = rm;
	}

	~ResourceManager();

	Handle<Texture> NewTexture();

	Material* NewMaterial();

	Texture* GetTexturePtr(Handle<Texture> texture);

	Handle<Texture> GetWhiteTexture();

	Handle<Texture> GetBlackTexture();

	Handle<Texture> GetErrorTexture();

private:

	struct
	{
		bool created = false;
		Handle<Texture> tex;
	} mWhiteTexture;

	struct
	{
		bool created = false;
		Handle<Texture> tex;
	} mBlackTexture;

	struct
	{
		bool created = false;
		Handle<Texture> tex;
	} mErrorTexture;

	RenderManager* mRenderManager;

	std::vector<Texture*> mLoadedTextures;

	std::vector<Material*> mMaterials;

};