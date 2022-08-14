#pragma once

#include "../../Vendor/ffx-cacao/ffx_cacao.h"
#include "../../Vendor/ffx-cacao/ffx_cacao_impl.h"

#include "../Vulkan/Device.h"

class CACAO
{
public:

	void Create(vk::Device* device);

	void SetUp(uint32_t w, uint32_t h, vk::Texture* depthImage, vk::Texture* normalTarget, vk::Texture* outputImage);

	void Execute(vk::CommandList* cmd, glm::mat4 proj, glm::mat4 view);


	void Destroy();

private:

	FFX_CACAO_VkContext* context;

	FFX_CACAO_VkScreenSizeInfo screenInfo{};

};