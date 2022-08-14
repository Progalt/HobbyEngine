#include "CACAOImpl.h"

void CACAO::Create(vk::Device* device)
{
	size_t ffxCacaoContextSize = FFX_CACAO_VkGetContextSize();
	context = (FFX_CACAO_VkContext*)malloc(ffxCacaoContextSize);
	if (!context)
		throw std::runtime_error("Failed to Allocate FidelityFX CACAO Context");
	FFX_CACAO_VkCreateInfo info = {};
	info.physicalDevice = device->GetPhysicalDevice();
	info.device = device->GetDevice();
	info.flags = FFX_CACAO_VK_CREATE_USE_DEBUG_MARKERS | FFX_CACAO_VK_CREATE_NAME_OBJECTS;
	FFX_CACAO_Status status = FFX_CACAO_VkInitContext(context, &info);

	if (status != FFX_CACAO_STATUS_OK)
		throw std::runtime_error("Failed to Init FidelityFX CACAO Vk Context");
}

void CACAO::SetUp(uint32_t w, uint32_t h, vk::Texture* depthImage, vk::Texture* normalTarget, vk::Texture* outputImage)
{
	screenInfo.width = w;
	screenInfo.height = h;
	screenInfo.depthView = depthImage->GetImageView();
	screenInfo.normalsView = nullptr;
	screenInfo.output = outputImage->GetImage();
	screenInfo.outputView = outputImage->GetImageView();
	screenInfo.useDownsampledSsao = true;
	FFX_CACAO_Status status = FFX_CACAO_VkInitScreenSizeDependentResources(context, &screenInfo);

	if (status != FFX_CACAO_STATUS_OK)
		throw std::runtime_error("Failed to Create FidelityFX CACAO Screen Resources");

	FFX_CACAO_Settings settings = FFX_CACAO_DEFAULT_SETTINGS;
	settings.generateNormals = true;
	status = FFX_CACAO_VkUpdateSettings(context, &settings);

	if (status != FFX_CACAO_STATUS_OK)
		throw std::runtime_error("Failed to Update FidelityFX CACAO Settings");
}



void CACAO::Execute(vk::CommandList* cmd, glm::mat4 proj, glm::mat4 view)
{
	glm::mat4 v = glm::transpose(glm::inverse(view));

	FFX_CACAO_Matrix4x4 p;
	FFX_CACAO_Matrix4x4 normalMat;
	for (uint32_t x = 0; x < 4; x++)
		for (uint32_t y = 0; y < 4; y++)
			p.elements[x][y] = proj[x][y];

	for (uint32_t x = 0; x < 4; x++)
		for (uint32_t y = 0; y < 4; y++)
			normalMat.elements[x][y] = v[x][y];

	FFX_CACAO_Status status = FFX_CACAO_VkDraw(context, cmd->GetCommandBuffer(), &p, &normalMat);
}

void CACAO::Destroy()
{

	FFX_CACAO_Status status = FFX_CACAO_VkDestroyScreenSizeDependentResources(context);

	if (status != FFX_CACAO_STATUS_OK)
		throw std::runtime_error("Failed to Destroy FidelityFX CACAO Screen Resources");

	status = FFX_CACAO_VkDestroyContext(context);

	if (status != FFX_CACAO_STATUS_OK)
		throw std::runtime_error("Failed to Destroy FidelityFX CACAO Vk Context");
}