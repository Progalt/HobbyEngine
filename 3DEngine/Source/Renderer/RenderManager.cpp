
#include "RenderManager.h"

#include "VkImpl/RenderManagerVk.h"

RenderManager* RenderManager::Create(Window* window, const RenderManagerCreateInfo& createInfo)
{
	return new RenderManagerVk(window, createInfo);
}

void RenderManager::Destroy(RenderManager* rm)
{
	((RenderManagerVk*)rm)->~RenderManagerVk();
	delete rm;
}