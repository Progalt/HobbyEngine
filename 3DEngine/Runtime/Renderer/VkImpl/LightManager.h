#pragma once

#include "../Vulkan/Device.h"
#include "../GraphicsStructs.h"

constexpr uint32_t MaxLightsPerScene = 1024;
constexpr uint32_t NumZSplits = 64;

// I've used this as inspiration for culling
// http://advances.realtimerendering.com/s2017/2017_Sig_Improved_Culling_final.pdf

// Lights are binned into xy frustums
// and then binned into a z bins. The Z splits are linear and completely seperate to the xy binning. 
// This removes the need for a depth buffer and removes the complexity of standard clustered light culling.

class LightManager
{
public:

	void Create(vk::Device* device);

	void Destroy();

	void UpdateLightBuffer(std::vector<PointLight> lights);


	vk::Buffer lightList;
	vk::Buffer zBin;
	vk::Buffer xyBin;


	vk::ComputePipeline compute;
	vk::DescriptorLayout layout;
	vk::Descriptor descriptor;

private:
};