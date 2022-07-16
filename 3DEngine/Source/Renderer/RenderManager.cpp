
#include "RenderManager.h"

#include "VkImpl/RenderManagerVk.h"

RenderManager* RenderManager::Create(Window* window)
{
	return new RenderManagerVk(window);
}

void RenderManager::Destroy(RenderManager* rm)
{
	delete rm;
}