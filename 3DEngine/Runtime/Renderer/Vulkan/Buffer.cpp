#include "Buffer.h"

#include "UtilityVK.h"
#include "Device.h"

namespace vk
{
	void Buffer::Create(BufferType type, BufferUsage usage, size_t size, void* data)
	{
		this->mSize = size;

		VkBufferUsageFlagBits bufferUsage;

		switch (usage)
		{
		case BufferUsage::Vertex: bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
		case BufferUsage::Index: bufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
		case BufferUsage::Uniform: bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
		case BufferUsage::Storage: bufferUsage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; break;
		case BufferUsage::Indirect: bufferUsage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT; break;
		}

		if (type == BufferType::Static)
		{
			/* Create a buffer that is only visible on GPU
			* this requires a CPU visible buffer to copy across to the GPU only buffer
			*/

			if (data == nullptr)
			{
				createBuffer(m_Allocator, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer, m_Allocation);
				return;
			}


			createBuffer(m_Allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingAlloc);

			vmaMapMemory(m_Allocator, stagingAlloc, &m_MappedBuffer);
			memcpy(m_MappedBuffer, data, size);
			vmaUnmapMemory(m_Allocator, stagingAlloc);

			m_Mapped = false;

			createBuffer(m_Allocator, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage, VMA_MEMORY_USAGE_GPU_ONLY, m_Buffer, m_Allocation);

			/* copy buffer across */

			SingleUseCommandBuffer cmd = m_Device->GetSingleUsageCommandBuffer(true);

			VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = 0;
			copyRegion.size = size;
			vkCmdCopyBuffer(cmd, stagingBuffer, m_Buffer, 1, &copyRegion);

			//m_Device->SubmitToList(cmd);

			m_Device->ExecuteTransfer(cmd);

			//m_Device->EndSingleUsageCmdBuffer(cmd, true, fence);

			//m_Device->WaitForFence(fence);

			destroyBuffer(m_Allocator, stagingBuffer, stagingAlloc);

		}
		else if (type == BufferType::Dynamic)
		{
			createBuffer(m_Allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage, VMA_MEMORY_USAGE_CPU_TO_GPU, m_Buffer, m_Allocation);

			vmaMapMemory(m_Allocator, m_Allocation, &m_MappedBuffer);

			m_Mapped = true;

			if (data)
				memcpy(m_MappedBuffer, data, size);
		}
	}

	void Buffer::SetData(size_t size, void* data, size_t offset)
	{
		//assert(m_Mapped);

		if (m_Mapped)
		{
			if (offset == 0)
			{
				memcpy(m_MappedBuffer, data, size);
			}
			else
			{
				void* buf = (void*)((size_t*)m_MappedBuffer + offset);

				memcpy(buf, data, size);
			}
		}
		else
		{
			//std::cout << "Copying using Staging Buffer\n";

			createBuffer(m_Allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, stagingBuffer, stagingAlloc);

			vmaMapMemory(m_Allocator, stagingAlloc, &m_MappedBuffer);
			memcpy(m_MappedBuffer, data, size);
			vmaUnmapMemory(m_Allocator, stagingAlloc);

			SingleUseCommandBuffer cmd = m_Device->GetSingleUsageCommandBuffer(true);

			VkBufferCopy copyRegion{};
			copyRegion.srcOffset = 0;
			copyRegion.dstOffset = offset;
			copyRegion.size = size;
			vkCmdCopyBuffer(cmd, stagingBuffer, m_Buffer, 1, &copyRegion);

			m_Device->ExecuteTransfer(cmd);

			destroyBuffer(m_Allocator, stagingBuffer, stagingAlloc);
		}
	}

	void Buffer::Destroy()
	{
		//if (stagingBuffer)
		//	destroyBuffer(m_Allocator, stagingBuffer, stagingAlloc);

		if (m_Mapped)
			vmaUnmapMemory(m_Allocator, m_Allocation);

		if (m_Buffer)
			destroyBuffer(m_Allocator, m_Buffer, m_Allocation);
	}
}