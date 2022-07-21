#pragma once

#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"

#include <vector>

namespace vk
{
	class Device;
	class CommandList;


	class Descriptor
	{
	public:


		void SetUpdateOnce();

		void BindBuffer(Buffer* buffer, uint32_t offset, uint32_t range, uint32_t binding, uint32_t arrayBinding = 0);

		void BindCombinedImageSampler(Texture* texture, Sampler* sampler, uint32_t binding, uint32_t arrayBinding = 0);

		void BindStorageBuffer(Buffer* buffer, uint32_t offset, uint32_t range, uint32_t binding, uint32_t arrayBinding = 0);

		void BindStorageImage(Texture* texture, uint32_t binding, uint32_t arrayBinding = 0);

		void Clear();

		void SetAutoUpdate(bool update);

		void Update();

	private:

		bool m_UpdateAll = true;

		bool m_AutoUpdate = false;

		VkDevice m_Device;

		Device* m_GraphicsDevice;

		void UpdateDescriptors();

		struct WriteInfo
		{
			VkDescriptorType type;

			uint32_t binding = 0;
			uint32_t arrayBinding = 0;

			union
			{
				VkDescriptorBufferInfo bufferInfo;
				VkDescriptorImageInfo imageInfo;
			};
		};

		struct Set
		{
			std::vector<WriteInfo> writes;

			//std::vector<VkWriteDescriptorSet> writes;
			VkDescriptorSet set;
		};


		std::vector<Set> m_Sets;

		friend Device;
		friend CommandList;


	};
}