
#include "LightProbeVk.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#include "../../FileSystem/FileSystem.h"

struct
{
	struct
	{
		vk::ComputePipeline pipeline;
		vk::DescriptorLayout layout;
	} genIrradiance;

	uint32_t refs = 0;
} shaders;

static std::vector<glm::mat4> matrices = {
	// POSITIVE_X
	glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
	// NEGATIVE_X
	glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
	// POSITIVE_Y
	glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
	// NEGATIVE_Y
	glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
	// POSITIVE_Z
	glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
	// NEGATIVE_Z
	glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
};

void LightProbeVk::Create(vk::Device* device, uint32_t baseRes, vk::DescriptorLayout* lightingLayout, vk::Sampler* cubeSampler)
{

	for (uint32_t i = 0; i < 6; i++)
		globalDataManager[i].Create(device, nullptr);

	// Create the texture

	cubemap = device->NewTexture();
	cubemap.CreateCubeMapTarget(vk::FORMAT_R16G16B16A16_SFLOAT, baseRes, vk::ImageLayout::General);

	irradiance = device->NewTexture();
	irradiance.CreateCubeMapTarget(vk::FORMAT_R16G16B16A16_SFLOAT, 32, vk::ImageLayout::ShaderReadOnlyOptimal);

	resolution = baseRes;


	if (shaders.refs == 0)
	{
		shaders.genIrradiance.layout = device->NewLayout();
		shaders.genIrradiance.layout.AddLayoutBinding({ 0, vk::ShaderInputType::StorageImage, 1, vk::ShaderStage::Compute });
		shaders.genIrradiance.layout.AddLayoutBinding({ 1, vk::ShaderInputType::StorageImage, 1, vk::ShaderStage::Compute });
		shaders.genIrradiance.layout.Create();

		vk::ShaderBlob blob = device->NewShaderBlob();
		blob.CreateFromSource(vk::ShaderStage::Compute, FileSystem::ReadBytes("Resources/Shaders/genIrradiance.comp.spv"));

		vk::ComputePipelineCreateInfo createInfo{};
		createInfo.layout = { &shaders.genIrradiance.layout };
		createInfo.computeBlob = &blob;

		shaders.genIrradiance.pipeline = device->NewComputePipeline(&createInfo);
	
		blob.Destroy();
	}

	shaders.refs++;

	irradianceDescriptor = device->NewDescriptor(&shaders.genIrradiance.layout);
	irradianceDescriptor.BindStorageImage(&cubemap, 0);
	irradianceDescriptor.BindStorageImage(&irradiance, 1);
	irradianceDescriptor.Update();

	lightingDescriptor = device->NewDescriptor(lightingLayout);
	lightingDescriptor.BindCombinedImageSampler(&irradiance, cubeSampler, 0);
	lightingDescriptor.Update();
}

void LightProbeVk::GenerateIrradiance(vk::CommandList& cmdList)
{
	vk::ImageBarrierInfo imgBarrier{};
	imgBarrier.srcAccess = vk::AccessFlags::ShaderRead;
	imgBarrier.oldLayout = vk::ImageLayout::ShaderReadOnlyOptimal;
	imgBarrier.dstAccess = vk::AccessFlags::ShaderWrite;
	imgBarrier.newLayout = vk::ImageLayout::General;

	cmdList.ImageBarrier(&irradiance, vk::PipelineStage::ComputeShader, vk::PipelineStage::ComputeShader, imgBarrier);

	// This is really slow at the moment
	cmdList.BindPipeline(&shaders.genIrradiance.pipeline);

	cmdList.BindDescriptors({ &irradianceDescriptor }, &shaders.genIrradiance.pipeline, 0);

	cmdList.Dispatch(resolution / 16, resolution / 16, 6);

	imgBarrier.srcAccess = vk::AccessFlags::ShaderWrite;
	imgBarrier.oldLayout = vk::ImageLayout::General;
	imgBarrier.dstAccess = vk::AccessFlags::ShaderRead;
	imgBarrier.newLayout = vk::ImageLayout::ShaderReadOnlyOptimal;

	cmdList.ImageBarrier(&irradiance, vk::PipelineStage::ComputeShader, vk::PipelineStage::ComputeShader, imgBarrier);
}

void LightProbeVk::UpdateData()
{
	for (uint32_t i = 0; i < 6; i++)
	{
		data[i].viewPos = glm::vec4(position, 1.0f);
		data[i].proj = glm::perspective(-(float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f);
		data[i].view = matrices[i] * glm::translate(glm::mat4(1.0f), position);
		data[i].VP = data[i].proj * data[i].view;
		data[i].jitteredVP = data[i].VP;
		data[i].prevVP = data[i].VP;
		data[i].inverseView = glm::inverse(data[i].view);
		data[i].inverseProj = glm::inverse(data[i].proj);

		globalDataManager[i].UpdateData(&data[i]);
	}
}

void LightProbeVk::Destroy()
{
	shaders.refs--;

	cubemap.Destroy();
	irradiance.Destroy();

	for (uint32_t i = 0; i < 6; i++)
		globalDataManager[i].Destroy();

	if (shaders.refs == 0)
	{
		shaders.genIrradiance.pipeline.Destroy();
		shaders.genIrradiance.layout.Destroy();
	}
}