
#include "LightManager.h"

void LightManager::Create(vk::Device* device)
{



}

void LightManager::Destroy()
{

}

void LightManager::UpdateLightBuffer(std::vector<PointLight> lights)
{

	lightList.SetData(sizeof(PointLight) * lights.size(), lights.data());
}