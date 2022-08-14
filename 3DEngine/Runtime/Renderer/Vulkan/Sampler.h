#pragma once
#include <vulkan/vulkan.h>

#include "Pipeline.h"

namespace vk
{
	class Imgui;

	enum class Filter
	{
		Nearest,
		Linear
	};

	enum class WrapMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder
	};


	struct SamplerSettings
	{
		Filter minFilter;
		Filter magFilter;

		WrapMode modeU;
		WrapMode modeV;
		WrapMode modeW;

		float minLod = 0.0f;
		float maxLod = 1000.0f;
		float mipLodBias = 0.0f;

		CompareOp compareOp = CompareOp::Less;
		bool compare = false;

		bool anisotropy = false;
		float maxAnisotropy = 1.0f;

	};

	class Descriptor;
	class Device;

	class Sampler
	{
	public:

		void Destroy();

	private:

		void Create(SamplerSettings* settings, VkDevice device);

		friend Descriptor;
		friend Device;
		friend Imgui;

		VkDevice m_Device;

		VkSampler m_Sampler;

	};
}