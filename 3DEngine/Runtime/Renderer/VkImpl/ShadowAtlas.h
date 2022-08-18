#pragma once

#include "../Vulkan/Device.h"
#include "../GraphicsStructs.h"

constexpr uint32_t CascadeSize_UltraLow = 512;
constexpr uint32_t CascadeSize_Low = 1024;
constexpr uint32_t CascadeSize_Medium = 2048;
constexpr uint32_t CascadeSize_High = 4096;
constexpr uint32_t CascadeCount = 3;

class CascadeShadowMap
{
public:

	void Create(vk::Device* device, QualitySetting quality);

	void Recreate(vk::Device* device, QualitySetting quality);

	void SetupForRendering(vk::CommandList& cmdList, uint32_t cascadeIndex);

	void FinishRendering(vk::CommandList& cmdList);

	void Destroy();

	void UpdateCascades(DirectionalLight& dirLight, float nearClip, float farClip,const glm::mat4& proj, const glm::mat4& view);

	void CreateDescriptor(vk::DescriptorLayout* layout, vk::Device* device);


	bool createdDescriptor = false;

	bool begunRenderpass = false;

	bool stabiliseCascades = false;

	struct DataBuffer
	{
		glm::mat4 matrices[CascadeCount];
		glm::vec4 splitDepths;
	} data;

	uint32_t size = 0;

	QualitySetting currentQuality = QualitySetting::Undefined;

	vk::Texture atlas;
	vk::Texture rt;

	vk::Renderpass renderpass;

	vk::Buffer uniformBuffer;

	vk::Descriptor descriptor;

private:
	
	glm::mat4 cachedView = glm::mat4(1.0f);

	uint32_t currentRenderCascade = 0;
};