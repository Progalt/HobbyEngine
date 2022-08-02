#pragma once

#include "../Resources/Model.h"

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