#pragma once

#include <glm/glm.hpp>

enum class CubeFace
{
	PositiveX,
	NegativeX,
	PositiveY,
	NegativeY,
	PositiveZ,
	NegativeZ
};


// A Light probe captures a stores the scene in a cubemap
class LightProbe
{
public:

	virtual void Destroy() = 0;

	glm::vec3 position = { 0.0f, 0.0f, 0.0f };

	// should the whole scene be rendered. If this is false only the sky will be rendered
	bool renderScene = true;

	// will be update next frame
	bool update  = true;

private:
};