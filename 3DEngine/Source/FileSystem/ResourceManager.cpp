
#include "ResourceManager.h"

#include "../Renderer/Material.h"

ResourceManager::~ResourceManager()
{
	for (auto& mat : mMaterials)
		delete mat;
}

Material* ResourceManager::GetMaterial(const std::string& name)
{
	for (auto& mat : mMaterials)
	{
		if (mat->name == name)
			return mat;
	}


	Material* m = new Material();
	m->name = name;

	mMaterials.push_back(m);
}