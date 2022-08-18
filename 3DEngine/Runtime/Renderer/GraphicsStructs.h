#pragma once

#include <glm/glm.hpp>

#define MAX_POINT_LIGHT_COUNT 512

enum class QualitySetting
{
	Undefined, 
	UltraLow,
	Low, 
	Medium,
	High
};

struct GlobalData
{
	glm::mat4 jitteredVP;
	glm::mat4 VP;
	glm::mat4 prevVP;
	glm::vec4 viewPos;
	glm::mat4 inverseProj;
	glm::mat4 inverseView;
	glm::mat4 view;
	glm::mat4 proj;
};

struct GPUPointLight
{
	glm::vec4 position;
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
	GPUPointLight pointLights[MAX_POINT_LIGHT_COUNT];
};

