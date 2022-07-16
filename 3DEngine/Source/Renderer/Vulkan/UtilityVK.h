#pragma once


#include <vulkan/vulkan.h>
#include "Vendor/vk_mem_alloc.h"

namespace vk
{
	void createBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memUsage, VkBuffer& buffer, VmaAllocation& bufferMemory);

	void destroyBuffer(VmaAllocator allocator, VkBuffer& buffer, VmaAllocation& bufferMemory);

	void createImage(VmaAllocator allocator, uint32_t width, uint32_t height, uint32_t depth, VkFormat format,
		VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VmaMemoryUsage memUsage, VkImage& image, VmaAllocation& allocation, 
		uint32_t mipLevels, uint32_t levels, VkImageCreateFlags createFlags);

	void destroyImage(VmaAllocator allocator, VkImage& image, VmaAllocation& alloc);

	void transitionImageLayout(VkCommandBuffer cmd, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	void copyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void generateMipmaps(VkCommandBuffer cmd, VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels, uint32_t layerCount);
}