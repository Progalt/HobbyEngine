#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include "Renderpass.h"

#include "Buffer.h"

#include <string>

#include "ComputePipeline.h"

namespace vk
{
	class Device;


	enum class CommandListType
	{
		Primary, Secondary
	};

	enum class IndexType
	{
		Uint16,
		Uint32
	};

	enum class AccessFlags
	{
		IndirectCommandRead = 0x00000001,
		IndexRight = 0x00000002,
		VertexAttributeRead = 0x00000004,
		UniformRead = 0x00000008,
		InputAttachmentRead = 0x00000010,
		ShaderRead = 0x00000020,
		ShaderWrite = 0x00000040,
		ColorAttachmentRead = 0x00000080,
		ColorAttachmentWrite = 0x00000100,
		DepthStencilAttachmentRead = 0x00000200,
		DepthAttachmentWrite = 0x00000400,
		TransferRead = 0x00000800,
		TransferWrite = 0x00001000,
		HostRead = 0x00002000,
		HostWrite = 0x00004000,
		MemoryRead = 0x00008000,
		MemoryWrite = 0x00010000,
	};

	enum class PipelineStage
	{
		TopOfPipe = 0x00000001,
		DrawIndirect = 0x00000002,
		VertexInput = 0x00000004,
		VertexShader = 0x00000008,
		//VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT = 0x00000010,
		//VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT = 0x00000020,
		//VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT = 0x00000040,
		FragmentShader = 0x00000080,
		EarlyFragmentTests = 0x00000100,
		LateFragmentTests = 0x00000200,
		ColorAttachmentOuput = 0x00000400,
		ComputeShader = 0x00000800,
		Transfer = 0x00001000,
		BottomOfPipe = 0x00002000,
		Host = 0x00004000,
		AllGraphics = 0x00008000,
		AllCommands = 0x00010000,
	};

	struct ImageBarrierInfo
	{
		AccessFlags srcAccess;
		AccessFlags dstAccess;
		ImageLayout oldLayout;
		ImageLayout newLayout;
	};

	struct DrawIndexedIndirectCommand
	{
		uint32_t    indexCount;
		uint32_t    instanceCount;
		uint32_t    firstIndex;
		int32_t     vertexOffset;
		uint32_t    firstInstance;
	};

	struct ImageCopy
	{
		int srcX, srcY, dstX, dstY;
		uint32_t w, h;
	};

	class CommandList
	{
	public:

		void Begin(Renderpass* inheritance = nullptr);

		void End();

		void BeginRenderpass(Renderpass* renderpass, bool hasSecondaryCmdLists, int32_t renderAreaW = -1, int32_t renderAreaH = -1);
		void EndRenderpass();

		void SetViewport(int x, int y, int w, int h, int minDepth = 0, int maxDepth = 1);

		void SetScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);

		void BindPipeline(Pipeline* pipeline);

		void BindPipeline(ComputePipeline* pipeline);

		void BindVertexBuffer(Buffer* buffer, uint32_t offset);

		void BindVertexBuffers(std::vector<Buffer*> buffers, std::vector<uint64_t> offsets);

		void BindIndexBuffer(Buffer* buffer, IndexType type = IndexType::Uint32);

		void BindDescriptors(std::vector<Descriptor*> descriptor, Pipeline* pipeline, uint32_t firstDescriptor);

		void BindDescriptors(std::vector<Descriptor*> descriptor, ComputePipeline* pipeline, uint32_t firstDescriptor);

		void Dispatch(uint32_t x, uint32_t y, uint32_t z);

		void PushConstants(vk::Pipeline* pipeline, ShaderStage stage, uint32_t size, uint32_t offset, void* data);

		void PushConstants(vk::ComputePipeline* pipeline, ShaderStage stage, uint32_t size, uint32_t offset, void* data);

		void ImageBarrier(Texture* texture, PipelineStage srcStage, PipelineStage dstStage, ImageBarrierInfo barrierInfo);

		void DrawIndexedIndirect(Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride);

		void CopyImage(Texture* src, ImageLayout srcLayout, Texture* dst, ImageLayout dstLayout, ImageCopy* region);

		void ClearColourImage(Texture* src, ImageLayout layout, glm::vec4 colour);

		void ClearDepth(float depthColour, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

		// VK_EXT_debug_utils
		void BeginDebugUtilsLabel(std::string name);

		// VK_EXT_debug_utils
		void EndDebugUtilsLabel();

		VkCommandBuffer GetCommandBuffer();

	private:

		friend Device;

		uint32_t threadNum = 0;

		Renderpass* m_Renderpass = nullptr;

		bool m_Secondary = false;

		friend Device;

		CommandListType type;

		Device* m_Device;

		uint32_t getIndex();

		std::vector<VkCommandBuffer> m_Cmd;

		

	};
}