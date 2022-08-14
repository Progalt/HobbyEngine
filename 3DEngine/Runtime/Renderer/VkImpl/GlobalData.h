#pragma once

#include "../Vulkan/Device.h"

#include "../GraphicsStructs.h"

class GlobalDataManager
{
public:

	void Destroy();

	void Create(vk::Device* device, GlobalData* data = nullptr);

	void UpdateData(GlobalData* data);

	vk::DescriptorLayout* GetLayout(vk::ShaderStage stage);

	vk::Descriptor* GetDescriptor(vk::ShaderStage stage);

private:

	uint32_t GetIndex(vk::ShaderStage stage)
	{
		switch (stage)
		{
		case vk::ShaderStage::Vertex: return 0; break;
		case vk::ShaderStage::Fragment: return 1; break;
		case vk::ShaderStage::Compute: return 2; break;
		}
	}

	vk::Buffer mBuffer;

	struct Stage
	{
		vk::Descriptor descriptor;
		vk::DescriptorLayout layout;
	};

	Stage mStages[3];
};