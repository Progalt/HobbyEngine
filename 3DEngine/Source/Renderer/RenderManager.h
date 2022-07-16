#pragma once

#include "Mesh.h"

#include "../Core/Window.h"


// Handles the rendering of the engine
class RenderManager
{
public:

	static RenderManager* Create(Window* window);

	static void Destroy(RenderManager* rm);

	virtual void QueueMesh(Mesh* mesh) = 0;

	virtual Mesh* NewMesh() = 0;

	virtual void Render() = 0;

	
};