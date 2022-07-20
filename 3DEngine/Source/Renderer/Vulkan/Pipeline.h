#pragma once

#include "DescriptorLayout.h"
#include "Renderpass.h"

#include "Format.h"
namespace vk
{
	class Device;
	class CommandList;
	class Pipeline;
	class ComputePipeline;

	struct BindingDescription
	{
		uint32_t binding = 0;
		uint32_t stride = 0;
	};

	struct AttributeDescription
	{
		uint32_t binding = 0;
		uint32_t location = 0;
		Format format;
		uint32_t offset = 0;
	};

	struct VertexDescription
	{
		std::vector<BindingDescription> bindingDescs;
		std::vector<AttributeDescription> attributeDescs;
	};

	class ShaderBlob
	{
	public:

		void CreateFromSource(ShaderStage stage, std::vector<int8_t> source);

		void Destroy();

	private:

		friend Device;
		friend Pipeline;
		friend ComputePipeline;

		ShaderStage stage;

		VkShaderModule m_Module;

		VkDevice m_Device;
	};

	struct PushConstantRange
	{
		PushConstantRange() { }
		PushConstantRange(ShaderStage stage, uint32_t offset, uint32_t size) : stage(stage), offset(offset), size(size) { }

		ShaderStage stage;
		uint32_t offset;
		uint32_t size;
	};

	enum class CullMode
	{
		None = 0,
		Front = 0x00000001,
		Back = 0x00000002,
		FrontAndBack = 0x00000003,
	};

	enum class Topology
	{
		PointList = 0,
		LineList = 1,
		LineStrip = 2,
		TriangleList = 3,
		TriangleStrip = 4,
		TriangleFan = 5,
		LineListWithAdjacency = 6,
		LineStripWithAdjacency = 7,
		TriangleListWithAdjacency = 8,
		TriangleStripWithAdjacency = 9,
		PatchList = 10,
	};

	enum class CompareOp
	{
		Never = 0,
		Less = 1,
		Equal = 2,
		LessOrEqual = 3,
		Greater = 4,
		NotEqual = 5,
		GreaterOrEqual = 6,
		Always = 7,
	};

	struct PipelineCreateInfo
	{
		std::vector<ShaderBlob*> shaders;
		Topology topologyType;
		CullMode cullMode;
		Renderpass* renderpass;
		VertexDescription* vertexDesc;
		std::vector<DescriptorLayout*> layout;
		std::vector<PushConstantRange> pushConstantRanges;
		bool blending = true;
		float lineWidth = 1.0f;
		bool depthTest = true;
		bool depthWrite = true;
		bool dynamicDepthBias = false;
		bool dynamicCullMode = false;
		CompareOp compareOp = CompareOp::Less;
	};

	class Pipeline
	{
	public:

		void Destroy();

	private:

		void Create(VkDevice device, PipelineCreateInfo* createInfo);

		friend Device;
		friend CommandList;

		VkDevice m_Device;

		VkPipeline m_Pipeline;
		VkPipelineLayout m_Layout;
	};
}