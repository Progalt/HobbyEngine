
#include "LightManager.h"

void LightManager::Create(vk::Device* device)
{



}

void LightManager::Destroy()
{

}

void LightManager::UpdateLightBuffer(std::vector<GPUPointLight> lights)
{

	lightList.SetData(sizeof(GPUPointLight) * lights.size(), lights.data());
}