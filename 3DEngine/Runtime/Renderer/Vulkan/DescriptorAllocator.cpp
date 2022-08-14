#include "DescriptorAllocator.h"

namespace vk
{
	VkDescriptorPool createPool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
	{
		std::vector<VkDescriptorPoolSize> sizes;
		sizes.reserve(poolSizes.sizes.size());
		for (auto sz : poolSizes.sizes)
		{
			sizes.push_back({ sz.first, uint32_t(sz.second * count) });
		}
		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = flags;
		pool_info.maxSets = count;
		pool_info.poolSizeCount = (uint32_t)sizes.size();
		pool_info.pPoolSizes = sizes.data();

		VkDescriptorPool descriptorPool;
		vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool);

		return descriptorPool;
	}

	void DescriptorAllocator::Reset()
	{
		for (auto p : m_UsedPools)
		{
			vkResetDescriptorPool(m_Device, p, 0);
		}

		m_FreePools = m_UsedPools;
		m_UsedPools.clear();

		m_CurrentPool = VK_NULL_HANDLE;
	}

	bool DescriptorAllocator::Allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
	{
		if (m_CurrentPool == VK_NULL_HANDLE)
		{
			m_CurrentPool = GrabPool();
			m_UsedPools.push_back(m_CurrentPool);
		}

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;

		allocInfo.pSetLayouts = &layout;
		allocInfo.descriptorPool = m_CurrentPool;
		allocInfo.descriptorSetCount = 1;

		//try to allocate the descriptor set
		VkResult allocResult = vkAllocateDescriptorSets(m_Device, &allocInfo, set);
		bool needReallocate = false;

		switch (allocResult)
		{
		case VK_SUCCESS:
			//all good, return
			return true;
		case VK_ERROR_FRAGMENTED_POOL:
		case VK_ERROR_OUT_OF_POOL_MEMORY:
			//reallocate pool
			needReallocate = true;
			break;
		default:
			//unrecoverable error
			return false;
		}

		if (needReallocate)
		{
			//allocate a new pool and retry
			m_CurrentPool = GrabPool();
			m_UsedPools.push_back(m_CurrentPool);

			allocResult = vkAllocateDescriptorSets(m_Device, &allocInfo, set);

			if (allocResult == VK_SUCCESS)
			{
				return true;
			}
		}

		return false;
	}



	void DescriptorAllocator::Init(VkDevice device)
	{
		m_Device = device;
	}

	void DescriptorAllocator::Cleanup()
	{
		for (auto p : m_FreePools)
		{
			vkDestroyDescriptorPool(m_Device, p, nullptr);
		}
		for (auto p : m_UsedPools)
		{
			vkDestroyDescriptorPool(m_Device, p, nullptr);
		}
	}

	VkDescriptorPool DescriptorAllocator::GrabPool()
	{
		if (m_FreePools.size() > 0)
		{

			VkDescriptorPool pool = m_FreePools.back();
			m_FreePools.pop_back();
			return pool;
		}
		else
		{
			//no pools availible, so create a new one
			return createPool(m_Device, m_PoolSizes, 10000, 0);
		}
	}
}