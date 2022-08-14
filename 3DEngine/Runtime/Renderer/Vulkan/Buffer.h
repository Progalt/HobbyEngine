#pragma once
#include <vulkan/vulkan.h>

#include "Vendor/vk_mem_alloc.h"

namespace vk
{
	class Device;
	class CommandList;
	class Descriptor;

	enum class BufferUsage
	{
		Vertex,
		Index,
		Uniform,
		Storage,
		Indirect
	};

	enum class BufferType
	{
		Static,
		Dynamic
	};

	class Buffer
	{
	public:

		void Create(BufferType type, BufferUsage usage, size_t size, void* data);

		void SetData(size_t size, void* data, size_t offset = 0);

		void Destroy();

		const size_t GetSize() const { return mSize; }

		bool Valid() {  if (m_Device == nullptr) { return false; } return true; }

	private:

		size_t mSize = 0;

		VkBuffer m_Buffer;

		VmaAllocation m_Allocation;

		void* m_MappedBuffer;

		bool m_Mapped = false;


		VkBuffer stagingBuffer;
		VmaAllocation stagingAlloc;

		VmaAllocator m_Allocator;

		Device* m_Device;

		friend Device;
		friend CommandList;
		friend Descriptor;

	};
}