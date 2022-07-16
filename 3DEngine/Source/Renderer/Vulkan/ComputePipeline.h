#pragma once


#include "DescriptorLayout.h"
#include "CommandList.h"

#include "Pipeline.h"

#include <vulkan/vulkan.h>

namespace vk
{
	class CommandList;
	class Device;

	struct ComputePipelineCreateInfo
	{
		ShaderBlob* computeBlob;
		std::vector<DescriptorLayout*> layout;
		std::vector<PushConstantRange> pushConstantRanges;
	};

	class ComputePipeline
	{
	public:

		

		void Destroy();

	private:

		friend CommandList;
		friend Device;

		void Create(VkDevice device, ComputePipelineCreateInfo* createInfo);

		VkDevice m_Device;

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_Layout = VK_NULL_HANDLE;

	};
}