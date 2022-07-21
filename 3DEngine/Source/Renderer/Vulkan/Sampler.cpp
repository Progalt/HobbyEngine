#include "Sampler.h"

#include <stdexcept>

namespace vk
{
	void Sampler::Create(SamplerSettings* settings, VkDevice device)
	{
		m_Device = device;

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		samplerInfo.minFilter = (VkFilter)settings->minFilter;
		samplerInfo.magFilter = (VkFilter)settings->magFilter;

		samplerInfo.addressModeU = (VkSamplerAddressMode)settings->modeU;
		samplerInfo.addressModeV = (VkSamplerAddressMode)settings->modeV;
		samplerInfo.addressModeW = (VkSamplerAddressMode)settings->modeW;


		samplerInfo.anisotropyEnable = settings->anisotropy;
		samplerInfo.maxAnisotropy = settings->maxAnisotropy;

		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		samplerInfo.compareEnable = settings->compare;
		samplerInfo.compareOp = (VkCompareOp)settings->compareOp;

		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = settings->mipLodBias;
		samplerInfo.minLod = settings->minLod;
		samplerInfo.maxLod = settings->maxLod;

		if (vkCreateSampler(m_Device, &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture sampler!");
		}

	}

	void Sampler::Destroy()
	{
		vkDestroySampler(m_Device, m_Sampler, nullptr);
	}
}