#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "../Core/Platform.h"

#include "Material.h"
#include "../Maths/BoundingBox.h"

class Mesh
{
public:

	Material* material;
	BoundingBox boundingBox;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;

	std::vector<IndexType> indices;

	virtual void GenerateMesh() = 0;

private:
};