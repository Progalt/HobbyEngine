#pragma once

#include "../FileSystem/ResourceManager.h"
#include <glm/glm.hpp>


class Material
{
public:

	virtual void Discard() = 0;

	Handle<Texture> albedo;
	glm::vec4 albedoColour;

	bool setUpdate = false;

private:
};