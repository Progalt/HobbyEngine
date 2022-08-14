#include "ComputePipeline.h"

#include <stdexcept>

namespace vk
{
	void ComputePipeline::Create(VkDevice device, ComputePipelineCreateInfo* createInfo)
	{
		m_Device = device;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		std::vector<VkDescriptorSetLayout> layouts;

		if (createInfo->layout.size() != 0)
		{

			layouts.resize(createInfo->layout.size());

			int i = 0;
			for (auto& x : layouts)
			{
				x = createInfo->layout[i]->m_SetLayout;
				i++;
			}

			pipelineLayoutInfo.setLayoutCount = layouts.size();
			pipelineLayoutInfo.pSetLayouts = layouts.data();
		}
		else
		{
			pipelineLayoutInfo.setLayoutCount = 0;
			pipelineLayoutInfo.pSetLayouts = nullptr;
		}

		std::vector<VkPushConstantRange> ranges;

		//std::cout << createInfo->pushConstantRanges.size() << "\n";

		if (createInfo->pushConstantRanges.size() != 0)
		{

			for (auto& range : createInfo->pushConstantRanges)
			{
				VkShaderStageFlags shaderStage;

				shaderStage = (VkShaderStageFlags)range.stage;

				VkPushConstantRange pcRange;
				pcRange.stageFlags = shaderStage;
				pcRange.offset = range.offset;
				pcRange.size = range.size;

				ranges.push_back(pcRange);
			}

			pipelineLayoutInfo.pushConstantRangeCount = ranges.size();
			pipelineLayoutInfo.pPushConstantRanges = ranges.data();

		}
		else
		{
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;
		}


		if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_Layout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}

		VkPipelineShaderStageCreateInfo stageInfo{};
		stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageInfo.pName = "main";
		stageInfo.module = createInfo->computeBlob->m_Module;
		stageInfo.pSpecializationInfo = 0;

		VkComputePipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = m_Layout;
		pipelineCreateInfo.stage = stageInfo;
		pipelineCreateInfo.basePipelineIndex = 0;
		pipelineCreateInfo.basePipelineHandle = 0;
		pipelineCreateInfo.flags = 0;
		pipelineCreateInfo.pNext = nullptr;

		if (vkCreateComputePipelines(m_Device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Compute pipeline");
		}
	}

	void ComputePipeline::Destroy()
	{
		vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);

		vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	}
}