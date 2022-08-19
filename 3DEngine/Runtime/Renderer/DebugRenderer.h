#pragma once

#include "../Core/Singleton.h"
#include <glm/glm.hpp>
#include <vector>

class DebugRenderer
{
public:

	DECLARE_SINGLETON(DebugRenderer)

	struct Vertex
	{
		glm::vec3 position;
		glm::vec4 colour;
	};

	std::vector<Vertex> vertices;

	void DrawLine(const glm::vec3& p1, const glm::vec3& p2, glm::vec4 colour);

	void DrawBox(const glm::vec3& min, const glm::vec3& max, glm::vec4 colour);


private:
};