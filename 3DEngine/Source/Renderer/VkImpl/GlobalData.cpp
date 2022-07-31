
#include "GlobalData.h"

void GlobalDataManager::Destroy()
{
	for (uint32_t i = 0; i < 3; i++)
	{
		mStages[i].layout.Destroy();
	}

	mBuffer.Destroy();
}

void GlobalDataManager::Create(vk::Device* device, GlobalData* data)
{
	mBuffer = device->NewBuffer();
	mBuffer.Create(vk::BufferType::Dynamic, vk::BufferUsage::Uniform, sizeof(GlobalData), data);

	for (uint32_t i = 0; i < 3; i++)
	{
		vk::ShaderStage currentStage;

		switch (i)
		{
		case 0: currentStage = vk::ShaderStage::Vertex; break;
		case 1: currentStage = vk::ShaderStage::Fragment; break;
		case 2: currentStage = vk::ShaderStage::Compute; break;
		}

		mStages[i].layout = device->NewLayout();
		mStages[i].layout.AddLayoutBinding({ 0, vk::ShaderInputType::UniformBuffer, 1, currentStage });
		mStages[i].layout.Create();

		mStages[i].descriptor = device->NewDescriptor(&mStages[i].layout);
		mStages[i].descriptor.BindBuffer(&mBuffer, 0, sizeof(GlobalData), 0);
		mStages[i].descriptor.Update();
	}
}

void GlobalDataManager::UpdateData(GlobalData* data)
{
	mBuffer.SetData(sizeof(GlobalData), data, 0);
}

vk::DescriptorLayout* GlobalDataManager::GetLayout(vk::ShaderStage stage)
{
	return &mStages[GetIndex(stage)].layout;
}

vk::Descriptor* GlobalDataManager::GetDescriptor(vk::ShaderStage stage)
{
	return &mStages[GetIndex(stage)].descriptor;
}