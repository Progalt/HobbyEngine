#include "Pipeline.h"

#include <stdexcept>

namespace vk
{
	void ShaderBlob::CreateFromSource(ShaderStage stage, std::vector<int8_t> source)
	{
		if (source.empty())
		{
			throw std::runtime_error("Shader source is empty");
		}

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = source.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(source.data());

		if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_Module) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Shader Blob");
		}

		this->stage = stage;
	}

	void ShaderBlob::Destroy()
	{
		vkDestroyShaderModule(m_Device, m_Module, nullptr);
	}


	VkVertexInputBindingDescription getBindingDesc(BindingDescription desc)
	{
		VkVertexInputBindingDescription output;

		output.binding = desc.binding;
		output.stride = desc.stride;
		output.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return output;
	}

	VkVertexInputAttributeDescription getAttributeDesc(AttributeDescription desc)
	{
		VkVertexInputAttributeDescription output;

		output.binding = desc.binding;
		output.location = desc.location;
		output.format = (VkFormat)desc.format;
		output.offset = desc.offset;

		return output;
	}

	void Pipeline::Create(VkDevice device, PipelineCreateInfo* createInfo)
	{
		m_Device = device;

		/* Gen Shader Stage Info */

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

		for (auto& shader : createInfo->shaders)
		{
			VkShaderStageFlagBits shaderStage = (VkShaderStageFlagBits)shader->stage;

		

			VkPipelineShaderStageCreateInfo stageInfo{};
			stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageInfo.stage = shaderStage;
			stageInfo.module = shader->m_Module;
			stageInfo.pName = "main";

			shaderStages.push_back(stageInfo);
		}

		VkPrimitiveTopology topology = (VkPrimitiveTopology)createInfo->topologyType;

		

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = topology;
		inputAssembly.primitiveRestartEnable = VK_FALSE;


		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		std::vector<VkVertexInputBindingDescription> bindingDesc;
		std::vector<VkVertexInputAttributeDescription> attrDesc;

		if (createInfo->vertexDesc != nullptr)
		{
			for (auto& desc : createInfo->vertexDesc->bindingDescs)
				bindingDesc.push_back(getBindingDesc(desc));

			for (auto& desc : createInfo->vertexDesc->attributeDescs)
				attrDesc.push_back(getAttributeDesc(desc));

			vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDesc.size());
			vertexInputInfo.pVertexBindingDescriptions = bindingDesc.data();
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDesc.size());
			vertexInputInfo.pVertexAttributeDescriptions = attrDesc.data();
		}
		else
		{
			vertexInputInfo.vertexBindingDescriptionCount = 0;
			vertexInputInfo.pVertexBindingDescriptions = nullptr;
			vertexInputInfo.vertexAttributeDescriptionCount = 0;
			vertexInputInfo.pVertexAttributeDescriptions = nullptr;
		}


		/* Setup rasterizer */

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = createInfo->lineWidth;


		VkCullModeFlags cullflags = (VkCullModeFlags)createInfo->cullMode;

	

		rasterizer.cullMode = cullflags;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

		rasterizer.depthBiasEnable = (createInfo->dynamicDepthBias) ? VK_TRUE : VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = (createInfo->blending) ? VK_TRUE : VK_FALSE;
		//colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = createInfo->renderpass->m_NumTargets;

		std::vector<VkPipelineColorBlendAttachmentState> bAttach;

		for (uint32_t i = 0; i < createInfo->renderpass->m_NumTargets; i++)
			bAttach.push_back(colorBlendAttachment);

		colorBlending.pAttachments = bAttach.data();
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

		if (createInfo->renderpass->hasDepth)
		{
			depthStencil.depthTestEnable = (VkBool32)createInfo->depthTest;
			depthStencil.depthWriteEnable = (VkBool32)createInfo->depthWrite;
			depthStencil.depthCompareOp = (VkCompareOp)createInfo->compareOp;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.minDepthBounds = 0.0f;
			depthStencil.maxDepthBounds = 1.0f;
			depthStencil.stencilTestEnable = VK_FALSE;
			depthStencil.front = {};
			depthStencil.back = {};
		}


		std::vector<VkDynamicState>dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_LINE_WIDTH,
			VK_DYNAMIC_STATE_DEPTH_BIAS
		};

		if (createInfo->dynamicCullMode)
		{
			dynamicStates.push_back(VK_DYNAMIC_STATE_CULL_MODE);
		}

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = dynamicStates.size();
		dynamicState.pDynamicStates = dynamicStates.data();;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)1.0f;
		viewport.height = (float)1.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { 1, 1 };

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;


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

			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
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
				VkShaderStageFlagBits shaderStage = (VkShaderStageFlagBits)range.stage;


				VkPushConstantRange pcRange;
				pcRange.stageFlags = shaderStage;
				pcRange.offset = range.offset;
				pcRange.size = range.size;

				ranges.push_back(pcRange);
			}

			pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(ranges.size());
			pipelineLayoutInfo.pPushConstantRanges = ranges.data();

		}
		else
		{
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;
		}


		if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_Layout) != VK_SUCCESS)
		{
			throw std::exception("Failed to create pipeline layout");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = (createInfo->renderpass->hasDepth) ? &depthStencil : nullptr;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = m_Layout;
		pipelineInfo.renderPass = createInfo->renderpass->m_Renderpass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
		{
			throw std::exception("Failed to create Pipeline");
		}
	}

	void Pipeline::Destroy()
	{
		vkDestroyPipelineLayout(m_Device, m_Layout, nullptr);
		vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
	}
}