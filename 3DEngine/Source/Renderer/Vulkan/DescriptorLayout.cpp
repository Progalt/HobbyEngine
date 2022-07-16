#include "DescriptorLayout.h"

#include <stdexcept>

namespace vk
{
	void DescriptorLayout::AddLayoutBinding(LayoutBinding binding)
	{
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = binding.binding;
		layoutBinding.descriptorCount = binding.count;
		layoutBinding.descriptorType = (VkDescriptorType)binding.inputType;
		layoutBinding.pImmutableSamplers = nullptr;

		VkShaderStageFlags shaderStage = (VkShaderStageFlags)binding.shaderStage;

		layoutBinding.stageFlags = shaderStage;

		if (binding.partialBind)
			m_PartialBinding = true;

		m_LayoutBindings.push_back(layoutBinding);

		m_Bindings.push_back(binding);
	}

	void DescriptorLayout::Create()
	{
		

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(m_LayoutBindings.size());
		layoutInfo.pBindings = m_LayoutBindings.data();

		VkDescriptorBindingFlags bindFlag = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;

		VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{};

		if (m_PartialBinding)
		{
	
			extendedInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			extendedInfo.pNext = nullptr;
			extendedInfo.bindingCount = static_cast<uint32_t>(m_LayoutBindings.size());;
			extendedInfo.pBindingFlags = &bindFlag;

			layoutInfo.pNext = &extendedInfo;
		}


		if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_SetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Pipline Input Layout");
		}
	}

	void DescriptorLayout::Destroy()
	{
		vkDestroyDescriptorSetLayout(m_Device, m_SetLayout, nullptr);
	}
}