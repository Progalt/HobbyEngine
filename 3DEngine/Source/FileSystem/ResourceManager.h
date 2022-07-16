#pragma once

#include <cstdint>

#include "../Core/Singleton.h"

#include "../Renderer/Texture.h"
#include <string>

class Material;

#include <vector>

template<typename _Ty>
class Handle
{
public:

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

	~ResourceManager();

	Handle<Texture> NewTexture();

	Material* GetMaterial(const std::string& name);

private:

	std::vector<Texture*> mLoadedTextures;

	std::vector<Material*> mMaterials;
};