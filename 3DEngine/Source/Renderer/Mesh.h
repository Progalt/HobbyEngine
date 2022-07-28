#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "../Core/Platform.h"

#include "Material.h"

#include "../Maths/BoundingBox.h"

class Mesh
{
public:

	//Material* material;

	BoundingBox boundingBox;

	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> texCoords;
	std::vector<glm::vec3> normals;

	std::vector<IndexType> indices;

	virtual void GenerateMesh() = 0;

	virtual void Destroy() = 0;

	void CalculateNormals()
	{
		normals.resize(positions.size());

		// Didn't know how to do it
		// Luckily this was useful
		// https://computergraphics.stackexchange.com/questions/4031/programmatically-generating-vertex-normals

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			glm::vec3 A = positions[indices[i]];
			glm::vec3 B = positions[indices[i + 1]];
			glm::vec3 C = positions[indices[i + 2]];

			glm::vec3 p = glm::cross(B - A, C - A);

			normals[indices[i]] += p;
			normals[indices[i + 1]] += p;
			normals[indices[i + 2]] += p;
		}

		for (size_t i = 0; i < normals.size(); i++)
			normals[i] = glm::normalize(normals[i]);
	}

private:
};