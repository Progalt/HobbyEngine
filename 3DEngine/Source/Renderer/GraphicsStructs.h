#pragma once

#include <glm/glm.hpp>

#define MAX_POINT_LIGHT_COUNT 512

struct GlobalData
{
	glm::mat4 jitteredVP;
	glm::mat4 VP;
	glm::mat4 prevVP;
	glm::vec4 viewPos;
};

struct PointLight
{
	glm::vec3 position;
	float radius;

	glm::vec4 colour;
};

struct DirectionalLight
{
	// Direction is a vec4 for padding reasons
	glm::vec4 direction;
	glm::vec4 colour;
};

struct SceneInfo
{
	uint32_t hasDirectionalLight;
	uint32_t lightCount;

	float padding[2];

	DirectionalLight dirLight;
	PointLight pointLights[MAX_POINT_LIGHT_COUNT];
};
