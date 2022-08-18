
#include "Descriptor.h"

#include "Device.h"

namespace vk
{
	void Descriptor::BindBuffer(Buffer* buffer, uint32_t offset, uint32_t range, uint32_t binding, uint32_t arrayBinding)
	{
		for (auto& set : m_Sets)
		{

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer->m_Buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = range;

			WriteInfo info;
			info.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			info.bufferInfo = bufferInfo;
			info.binding = binding;
			info.arrayBinding = arrayBinding;

			set.writes.push_back(info);

		}

		if (m_AutoUpdate)
			UpdateDescriptors();
	}

	void Descriptor::BindCombinedImageSampler(Texture* texture, Sampler* sampler, uint32_t binding, uint32_t arrayBinding)
	{

		if (texture == nullptr)
			return;

		for (auto& set : m_Sets)
		{


			if (!texture->m_Image || !texture->m_ImageView)
				return;

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture->m_ImageView;
			imageInfo.sampler = sampler->m_Sampler;

			WriteInfo info;
			info.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			info.imageInfo = imageInfo;
			info.binding = binding;
			info.arrayBinding = arrayBinding;

			set.writes.push_back(info);

		}

		if (m_AutoUpdate)
			UpdateDescriptors();
	}

	void Descriptor::BindStorageBuffer(Buffer* buffer, uint32_t offset, uint32_t range, uint32_t binding, uint32_t arrayBinding)
	{
		for (auto& set : m_Sets)
		{

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer->m_Buffer;
			bufferInfo.offset = offset;
			bufferInfo.range = range;

			WriteInfo info;
			info.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			info.bufferInfo = bufferInfo;
			info.binding = binding;
			info.arrayBinding = arrayBinding;

			set.writes.push_back(info);

		}

		if (m_AutoUpdate)
			UpdateDescriptors();
	}

	void Descriptor::BindStorageImage(Texture* texture, uint32_t binding, uint32_t arrayBinding)
	{
		if (texture == nullptr)
			return;

		for (auto& set : m_Sets)
		{


			if (!texture->m_Image || !texture->m_ImageView)
				return;

			VkDescriptorImageInfo imageInfo{};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageInfo.imageView = texture->m_ImageView;
			imageInfo.sampler = nullptr;

			WriteInfo info;
			info.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			info.imageInfo = imageInfo;
			info.binding = binding;
			info.arrayBinding = arrayBinding;

			set.writes.push_back(info);

		}

		if (m_AutoUpdate)
			UpdateDescriptors();
	}

	void Descriptor::Clear()
	{
		for (auto& set : m_Sets)
		{
			set.writes.clear();
		}
	}

	void Descriptor::SetUpdateOnce()
	{
		m_UpdateAll = true;
	}

	void Descriptor::SetAutoUpdate(bool update)
	{
		m_AutoUpdate = update;
	}

	void Descriptor::Update()
	{
		UpdateDescriptors();
	}

	void Descriptor::UpdateDescriptors()
	{

		for (auto& set : m_Sets)
		{
			std::vector<VkWriteDescriptorSet> writes;
			writes.resize(set.writes.size());

			int i = 0;
			for (auto& w : set.writes)
			{

				writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writes[i].dstSet = set.set;
				writes[i].dstBinding = w.binding;
				writes[i].dstArrayElement = w.arrayBinding;
				writes[i].descriptorType = w.type;
				writes[i].descriptorCount = 1;
				switch (w.type)
				{
				case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:	writes[i].pImageInfo = &w.imageInfo; break;
				case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:	writes[i].pImageInfo = &w.imageInfo; break;
				case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:	writes[i].pBufferInfo = &w.bufferInfo; break;
				case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER: writes[i].pBufferInfo = &w.bufferInfo;
				}

				i++;
			}


			vkUpdateDescriptorSets(m_Device, writes.size(), writes.data(), 0, nullptr);
		}

	}
}