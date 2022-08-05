
#include "PostProcessEffectVk.h"

#include "RenderManagerVk.h"


PostProcessEffectVk::PostProcessEffectVk(RenderManagerVk* renderManager, const PostProcessCreateInfo& createInfo)
{
	this->createInfo = createInfo;

	descriptorLayout = renderManager->mDevice.NewLayout();
	uint32_t currentOffset = 0;
	descriptorLayout.AddLayoutBinding({ currentOffset, vk::ShaderInputType::StorageImage, 1, vk::ShaderStage::Compute });
	currentOffset++;
	if (createInfo.uniformBufferSize != 0)
	{
		descriptorLayout.AddLayoutBinding({ currentOffset, vk::ShaderInputType::UniformBuffer, 1, vk::ShaderStage::Compute });
		currentOffset++;
	}

	for (auto& type : createInfo.inputs)
	{
		descriptorLayout.AddLayoutBinding({ currentOffset, vk::ShaderInputType::ImageSampler, 1, vk::ShaderStage::Compute });

		currentOffset++;
	}

	descriptorLayout.Create();

	descriptor = renderManager->mDevice.NewDescriptor(&descriptorLayout);

	vk::ShaderBlob blob = renderManager->mDevice.NewShaderBlob();
	blob.CreateFromSource(vk::ShaderStage::Compute, createInfo.shaderByteCode);

	vk::ComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo.computeBlob = &blob;

	if (createInfo.passGlobalData)
		pipelineInfo.layout.push_back(renderManager->globalDataManager.GetLayout(vk::ShaderStage::Compute));

	pipelineInfo.layout.push_back(&descriptorLayout);

	computePipeline = renderManager->mDevice.NewComputePipeline(&pipelineInfo);

	blob.Destroy();

	computeShader = true;

	if (createInfo.uniformBufferSize != 0)
	{
		uniformBuffer = renderManager->mDevice.NewBuffer();
		uniformBuffer.Create(vk::BufferType::Dynamic, vk::BufferUsage::Uniform, createInfo.uniformBufferSize, nullptr);
	}

	this->rm = renderManager;
}

void PostProcessEffectVk::UpdateUniformBuffer(void* data)
{
	if (createInfo.uniformBufferSize != 0)
		uniformBuffer.SetData(createInfo.uniformBufferSize, data);
}

void PostProcessEffectVk::Destroy()
{
	if (createInfo.uniformBufferSize != 0)
		uniformBuffer.Destroy();
	computePipeline.Destroy();
	descriptorLayout.Destroy();
}

void PostProcessEffectVk::GenerateDescriptor(vk::Texture* target, vk::Texture* currentColour)
{
	descriptor.Clear();

	descriptor.BindStorageImage(target, 0);
	uint32_t currentOffset = 1;

	if (createInfo.uniformBufferSize != 0)
	{
		descriptor.BindBuffer(&uniformBuffer, 0, createInfo.uniformBufferSize, currentOffset);
		currentOffset++;
	}


	for (auto& type : createInfo.inputs)
	{

		switch (type)
		{
		case PostProcessInput::Colour:
			descriptor.BindCombinedImageSampler(currentColour, &rm->mTargetSampler, currentOffset);
			break;
		case PostProcessInput::Depth:
			descriptor.BindCombinedImageSampler(&rm->mGeometryPass.depthTarget, &rm->mTargetSampler, currentOffset);
			break;
		case PostProcessInput::Normal_Roughness_Metallic:
			descriptor.BindCombinedImageSampler(&rm->mGeometryPass.normalTarget, &rm->mTargetSampler, currentOffset);
			break;
		case PostProcessInput::Velocity:
			descriptor.BindCombinedImageSampler(&rm->mGeometryPass.velocityTarget, &rm->mTargetSampler, currentOffset);
			break;
		case PostProcessInput::Emissive:
			descriptor.BindCombinedImageSampler(&rm->mGeometryPass.emissiveTarget, &rm->mTargetSampler, currentOffset);
			break;
		case PostProcessInput::History:
			descriptor.BindCombinedImageSampler(&rm->mHistory, &rm->mTargetSampler, currentOffset);
			break;
		}


		currentOffset++;
	}

	descriptor.Update();
}