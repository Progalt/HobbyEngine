#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace vk
{
	class DescriptorAllocator
	{
	public:

		struct PoolSizes
		{
			std::vector<std::pair<VkDescriptorType, float>> sizes =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
			};
		};

		void Reset();

		bool Allocate(VkDescriptorSet * set, VkDescriptorSetLayout layout);

		void Free(VkDescriptorSet * set);

		void Init(VkDevice device);

		void Cleanup();

	private:

		VkDevice m_Device;

		VkDescriptorPool GrabPool();

		VkDescriptorPool m_CurrentPool = VK_NULL_HANDLE;

		PoolSizes m_PoolSizes;

		std::vector<VkDescriptorPool> m_FreePools;
		std::vector<VkDescriptorPool> m_UsedPools;

	};
}