#include "Texture.h"

#include "Device.h"

#include "UtilityVK.h"

namespace vk
{


	void Texture::Create(TextureType type, Format format, uint32_t width, uint32_t height, void* pixels, uint32_t levels, uint32_t mipLevels, bool cubemap, uint32_t bytesPerPixel)
	{
		if (m_Allocator == nullptr || m_Device == nullptr)
			throw std::runtime_error("Allocator/Device is nullptr");

		uint32_t mips = (mipLevels == 0) ? 1 : mipLevels;

		VkImageType imageType = (VkImageType)type;

		this->format = format;

		VmaMemoryUsage memUsage = (pixels) ? VMA_MEMORY_USAGE_GPU_ONLY : VMA_MEMORY_USAGE_CPU_TO_GPU;

		int usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		int flags = 0;

		if (cubemap)
			flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		createImage(m_Allocator, width, height, 1, (VkFormat)format, imageType, VK_IMAGE_TILING_OPTIMAL,
			usageFlags, memUsage, m_Image, m_Allocation, mips, levels, flags);


		if (pixels)
		{
			VkDeviceSize imageSize = width * height * bytesPerPixel;


			createBuffer(m_Allocator, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingAlloc);

			void* data;
			if (vmaMapMemory(m_Allocator, stagingAlloc, &data) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to map memory");
			}
			memcpy(data, pixels, imageSize);
			vmaUnmapMemory(m_Allocator, stagingAlloc);

			// Copy buffer to texture

			SingleUseCommandBuffer cmd = m_Device->GetSingleUsageCommandBuffer(true);

			transitionImageLayout(cmd, m_Image, (VkFormat)format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mips);

			copyBufferToImage(cmd, stagingBuffer, m_Image, width, height);

			//transitionImageLayout(cmd, m_Image, (VkFormat)format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

			m_CurrentLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			m_Device->ExecuteTransfer(cmd);

			SingleUseCommandBuffer gfxCmd = m_Device->GetSingleUsageCommandBuffer(false);

			switch (mips > 1)
			{
			case true:
				generateMipmaps(gfxCmd, m_Image, width, height, mips, levels);
				break;
			case false:
				transitionImageLayout(gfxCmd, m_Image, (VkFormat)format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mips);
				break;
			}

			m_Device->ExecuteTransfer(gfxCmd);

			destroyBuffer(m_Allocator, stagingBuffer, stagingAlloc);
		}

		//std::cout << "Creating Texture\n";

		bool depthImage = false;

		VkFormat vkf = (VkFormat)format;

		if (vkf == VK_FORMAT_D16_UNORM || vkf == VK_FORMAT_D16_UNORM_S8_UINT ||
			vkf == VK_FORMAT_D24_UNORM_S8_UINT || vkf == VK_FORMAT_D32_SFLOAT ||
			vkf == VK_FORMAT_D32_SFLOAT_S8_UINT)
		{
			depthImage = true;
		}


		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = (VkFormat)format;
		viewInfo.subresourceRange.aspectMask = (depthImage) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mips;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = levels;

		m_ResourceRange = viewInfo.subresourceRange;

		if (vkCreateImageView(m_Device->m_Device, &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view");
		}

		m_Width = width;
		m_Height = height;
		m_MipLevels = mipLevels;
	
		static uint32_t textureCount = 1;

		m_Id = textureCount++;

	}

	void Texture::CreateRenderTarget(Format format, uint32_t width, uint32_t height, bool storageImage, bool transitionToShaderResource)
	{
		VkFormat vkf = (VkFormat)format;

		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		VkImageLayout targetLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		this->format = format;

		bool depthImage = false;

		if (vkf == VK_FORMAT_D16_UNORM || vkf == VK_FORMAT_D16_UNORM_S8_UINT ||
			vkf == VK_FORMAT_D24_UNORM_S8_UINT || vkf == VK_FORMAT_D32_SFLOAT ||
			vkf == VK_FORMAT_D32_SFLOAT_S8_UINT)
		{
			depthImage = true;

			usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			targetLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		}

		usageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		if (storageImage)
		{
			usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
		}

		createImage(m_Allocator, width, height, 1, (VkFormat)format, VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL,
			usageFlags, VMA_MEMORY_USAGE_GPU_ONLY, m_Image, m_Allocation, 1, 1, 0);

		m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_Image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = (VkFormat)format;
		viewInfo.subresourceRange.aspectMask = (depthImage) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		m_ResourceRange = viewInfo.subresourceRange;

		m_Width = width;
		m_Height = height;
		m_MipLevels = 1;

		if (vkCreateImageView(m_Device->m_Device, &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture image view");
		}

		if (transitionToShaderResource)
		{
			SingleUseCommandBuffer cmd = m_Device->GetSingleUsageCommandBuffer(false);

			transitionImageLayout(cmd, m_Image, (VkFormat)format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);

			m_Device->ExecuteTransfer(cmd);
		}
	}


	void Texture::Destroy()
	{
		if (m_ImageView)
			vkDestroyImageView(m_Device->m_Device, m_ImageView, nullptr);

		if (m_Image)
			destroyImage(m_Allocator, m_Image, m_Allocation);
	}
}