#pragma once

#include "../LightProbe.h"
#include "../Vulkan/Device.h"
#include "GlobalData.h"

constexpr uint32_t SphericalHarmonicsCoefficients = 9;

class LightProbeVk : public LightProbe
{
public:

	void Create(vk::Device* device, uint32_t baseRes, vk::DescriptorLayout* lightingLayout, vk::Sampler* cubeSampler);

	void UpdateData();

	void Destroy() override;

	void GenerateIrradiance(vk::CommandList& cmdList);

	vk::Descriptor lightingDescriptor;

	vk::Texture cubemap;

	vk::Texture irradiance;

	vk::Descriptor irradianceDescriptor;

	uint32_t resolution = 0;

	// One for each face
	GlobalData data[6];
	GlobalDataManager globalDataManager[6];

private:
};