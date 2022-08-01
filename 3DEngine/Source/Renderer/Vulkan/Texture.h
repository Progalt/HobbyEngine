#pragma once

#include <vulkan/vulkan.h>

#include "Vendor/vk_mem_alloc.h"

#include "Format.h"

#include <cmath>
#include <algorithm>
#undef max

namespace vk
{
	enum class ImageLayout
	{
		Undefined = 0,
		General = 1,
		ColourAttachmentOptimal = 2,
		DepthStencilAttachmentOptimal = 3,
		DepthStencilReadOnlyOptimal = 4,
		ShaderReadOnlyOptimal = 5,
		TransferSrcOptimal = 6,
		TransferDstOptimal = 7,
		PreInitilised = 8,
	};

	enum class TextureType
	{
		e1D,
		e2D,
		e3D
	};

	class Device;
	class Descriptor;
	class CommandList;
	class Imgui;

	inline uint32_t CalculateMipLevels(uint32_t w, uint32_t h)
	{
		return (static_cast<uint32_t>(std::floor(std::log2(std::max(w, h)))) + 1u);
	}

	class Texture
	{
	public:

		void Create(TextureType type, Format format, uint32_t width, uint32_t height, void* pixels, uint32_t levels = 1, uint32_t mipLevels = 1, bool cubemap = false, uint32_t bytesPerPixel = 4);

		void CreateRenderTarget(Format format, uint32_t width, uint32_t height, bool storageImage = false,  vk::ImageLayout transitionTo = vk::ImageLayout::Undefined, uint32_t levels = 1);

		void Destroy();

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		VkImage GetRaw() const { return m_Image; }

		bool operator==(Texture& tex)
		{
			if (m_Id == tex.m_Id)
				return true;

			return false;
		}

		VkImage GetImage() { return m_Image; }
		VkImageView GetImageView() { return m_ImageView; }


	private:

		friend Device;
		friend Descriptor;
		friend CommandList;
		friend Imgui;

		uint32_t m_Id = 0;

		VkBuffer stagingBuffer;
		VmaAllocation stagingAlloc;

		VkImage m_Image;
		VkImageView m_ImageView;
		VmaAllocation m_Allocation;

		VkImageLayout m_CurrentLayout;

		VmaAllocator m_Allocator;
		Device* m_Device;

		Format format;

		uint32_t m_Width = 0, m_Height;

		uint32_t m_MipLevels = 0;

		VkImageSubresourceRange m_ResourceRange;

	};
}