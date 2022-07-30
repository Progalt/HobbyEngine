#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

class BoundingBox
{
public:

	glm::vec3 min, max;

	glm::vec3 untransformed_min, untransformed_max;

	void Transform(const glm::mat4& model)
	{
		min = model * glm::vec4(untransformed_min, 1.0f);
		max = model * glm::vec4(untransformed_max, 1.0f);
	}

	// Calculates the untranslated bounding box of the mesh from its vertices
	void CalculateFromVertexData(std::vector<glm::vec3> positions)
	{
		auto xExtremes = std::minmax_element(positions.begin(), positions.end(),
			[](const glm::vec3& lhs, const glm::vec3& rhs) {
				return lhs.x < rhs.x;
			});

		auto yExtremes = std::minmax_element(positions.begin(), positions.end(),
			[](const glm::vec3& lhs, const glm::vec3& rhs) {
				return lhs.y < rhs.y;
			});

		auto zExtremes = std::minmax_element(positions.begin(), positions.end(),
			[](const glm::vec3& lhs, const glm::vec3& rhs) {
				return lhs.z < rhs.z;
			});


		min = { xExtremes.first->x, yExtremes.first->y, zExtremes.first->z };
		max = { xExtremes.second->x, yExtremes.second->y, zExtremes.second->z };

		untransformed_min = min;
		untransformed_max = max;
	}

private:
};