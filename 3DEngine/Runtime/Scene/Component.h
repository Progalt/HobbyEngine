#pragma once

#include "../Resources/Model.h"
#include "../Core/GlobalState.h"
#include "../Maths/Frustum.h"
#include <glm/gtc/matrix_transform.hpp>

class Actor;

class Component
{
public:


	virtual void Start() { }

	virtual void Tick(const float dt) { }

	virtual void Destroy() { }

	void SetParentActor(Actor* actor)
	{
		mParent = actor;
	}

private:

	Actor* mParent;
};



// Rendering Component

class MeshRenderer : public Component
{
public:

	Model* model;

private:
};

class PointLight : public Component
{
public:

	float radius;
	float intensity;
	glm::vec3 colour;

private:
};


class PerspectiveCamera : public Component
{
public:

	struct
	{
		// Specified in degrees
		float fov = 90.0f;

		float nearPlane = 0.1f;

		float farPlane = 1000.0f;

	} settings;

	void ConstructProjection()
	{
		float aspectRatio = (float)GlobalState::GetInstance().width / (float)GlobalState::GetInstance().height;
		projection_reversedDepth = ReversedDepthPerspective(glm::radians(settings.fov), aspectRatio, settings.nearPlane);
		projection = glm::perspective(glm::radians(settings.fov), aspectRatio, settings.nearPlane, settings.farPlane);


		//projection_reversedDepth[1][1]  *= -1.0f;
		//projection[1][1] *= -1.0f;

	}

	glm::mat4 projection_reversedDepth;
	glm::mat4 projection;

private:
};