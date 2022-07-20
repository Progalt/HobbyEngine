#pragma once

#include "../FileSystem/ResourceManager.h"
#include <glm/glm.hpp>


class Material
{
public:

	Handle<Texture> albedo;
	glm::vec4 albedoColour;

private:
};