#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace vk
{
	class Pipeline;
	class Device;
	class CommandList;
	class ComputePipeline;

	enum class ShaderInputType
	{
		Sampler = 0,
		ImageSampler = 1,
		SampledImage = 2,
		StorageImage = 3,
		UniformTexelBuffer = 4,
		StorageTexelBuffer = 5,
		UniformBuffer = 6,
		StorageBuffer = 7,
		UniformBufferDynamic = 8,
		StorageBufferDynamic = 9,
		InputAttachment = 10,
		MaxEnum = 0x7FFFFFFF
	};

	enum class ShaderStage
	{
		Vertex = 0x00000001,
		Control = 0x00000002,
		TesselationEvalutation = 0x00000004,
		Geometry = 0x00000008,
		Fragment = 0x00000010,
		Compute = 0x00000020,
		AllGraphics = 0x0000001F,
		All = 0x7FFFFFFF,
	};

	struct LayoutBinding
	{
		uint32_t binding;
		ShaderInputType inputType;
		uint32_t count;
		ShaderStage shaderStage;
		bool partialBind = false;
	};

	class DescriptorLayout
	{
	public:

		void AddLayoutBinding(LayoutBinding binding);

		void Create();

		void Destroy();

	private:

		friend Pipeline;
		friend Device;
		friend CommandList;
		friend ComputePipeline;

		VkDescriptorSetLayout m_SetLayout;

		std::vector<LayoutBinding> m_Bindings;

		bool m_PartialBinding = false;

		VkDevice m_Device;

		std::vector<VkDescriptorSetLayoutBinding> m_LayoutBindings;

	};
}