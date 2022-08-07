#include "CommandList.h"
#include "Device.h"

#include <iostream>

namespace vk
{
	void CommandList::Begin(Renderpass* inheritance)
	{
		VkCommandBufferInheritanceInfo inheritanceInfo{};

		if (inheritance != nullptr)
		{

			inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			inheritanceInfo.renderPass = inheritance->m_Renderpass;
			inheritanceInfo.subpass = 0;
			inheritanceInfo.framebuffer = inheritance->m_Framebuffers[((inheritance->type == RenderpassType::Swapchain) ? m_Device->m_CurrentFrame : 0)];
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (inheritance != nullptr)
			beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

		beginInfo.pInheritanceInfo = (inheritance != nullptr) ? &inheritanceInfo : nullptr;

		if (vkBeginCommandBuffer(m_Cmd[getIndex()], &beginInfo) != VK_SUCCESS)
		{
			throw std::exception("Failed to begin command list");
		}
	}

	void CommandList::End()
	{
		if (vkEndCommandBuffer(m_Cmd[getIndex()]) != VK_SUCCESS)
		{
			throw std::exception("Failed to end command list");
		}
	}

	void CommandList::BeginRenderpass(Renderpass* renderpass, bool hasSecondaryCmdLists, int32_t renderAreaW, int32_t renderAreaH)
	{
		Renderpass* rp = renderpass;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = rp->m_Renderpass;
		renderPassInfo.framebuffer = rp->m_Framebuffers[((renderpass->type == RenderpassType::Swapchain) ? m_Device->m_CurrentFrame : 0)];

		renderPassInfo.renderArea.offset = { 0, 0 };

		VkExtent2D extent;
#ifdef VK_DONT_USE_BOOTSTRAP
		extent.width = (renderAreaW == -1) ? m_Device->m_Swapchain.getExtent().width : renderAreaW;
		extent.height = (renderAreaH == -1) ? m_Device->m_Swapchain.getExtent().height : renderAreaH;
#else 
		extent.width = (renderAreaW == -1) ? m_Device->m_Swapchain.extent.width : renderAreaW;
		extent.height = (renderAreaH == -1) ? m_Device->m_Swapchain.extent.height : renderAreaH;
#endif

		renderPassInfo.renderArea.extent = extent;

		glm::vec4 col = renderpass->clearColour;

		std::vector<VkClearValue> clearValues;

		if (rp->m_NumTargets > 0)
			clearValues.push_back({ col.r, col.g, col.b , col.a });

		if (rp->m_NumTargets > 0)
			for (uint32_t i = 0; i < rp->m_NumTargets - 1; i++)
				clearValues.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });

		if (renderpass->hasDepth)
			clearValues.push_back({ renderpass->depthClearColour, 0 });

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		switch (hasSecondaryCmdLists)
		{
		case true:
			vkCmdBeginRenderPass(m_Cmd[getIndex()], &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			break;
		case false:
			vkCmdBeginRenderPass(m_Cmd[getIndex()], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			break;
		}

		m_Renderpass = rp;
	}

	void CommandList::EndRenderpass()
	{
		vkCmdEndRenderPass(m_Cmd[getIndex()]);

		m_Renderpass = nullptr;
	}

	void CommandList::SetViewport(int x, int y, int w, int h, int minDepth, int maxDepth)
	{
		VkViewport viewport;
		viewport.width = (float)w;
		viewport.height = (float)h;
		viewport.x = (float)x;
		viewport.y = (float)y;
		viewport.minDepth = (float)minDepth;
		viewport.maxDepth = (float)maxDepth;

		vkCmdSetViewport(m_Cmd[getIndex()], 0, 1, &viewport);
	}

	void CommandList::SetScissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
	{
		VkRect2D rect;
		rect.offset = { (int32_t)x, (int32_t)y };
		rect.extent = { w, h };

		vkCmdSetScissor(m_Cmd[getIndex()], 0, 1, &rect);
	}

	void CommandList::BindPipeline(Pipeline* pipeline)
	{

		vkCmdBindPipeline(m_Cmd[getIndex()], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_Pipeline);

	}

	void CommandList::BindPipeline(ComputePipeline* pipeline)
	{
		vkCmdBindPipeline(m_Cmd[getIndex()], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->m_Pipeline);
	}

	void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	{
		vkCmdDraw(m_Cmd[getIndex()], vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
	{
		vkCmdDrawIndexed(m_Cmd[getIndex()], indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void CommandList::BindVertexBuffer(Buffer* buffer, uint32_t offset)
	{
		VkBuffer vertexBuffers[] = { buffer->m_Buffer };
		VkDeviceSize offsets[] = { offset };

		vkCmdBindVertexBuffers(m_Cmd[getIndex()], 0, 1, vertexBuffers, offsets);
	}

	void CommandList::BindVertexBuffers(std::vector<Buffer*> buffers, std::vector<uint64_t> offsets)
	{
		std::vector<VkBuffer> bufs(buffers.size());
		for (uint32_t i = 0; i < buffers.size(); i++)
		{
			bufs[i] = buffers[i]->m_Buffer;
		}

		vkCmdBindVertexBuffers(m_Cmd[getIndex()], 0, buffers.size(), bufs.data(), offsets.data());
		
	}

	void CommandList::BindIndexBuffer(Buffer* buffer, IndexType type)
	{
		vkCmdBindIndexBuffer(m_Cmd[getIndex()], buffer->m_Buffer, 0, (VkIndexType)type);
	}

	void CommandList::BindDescriptors(std::vector<Descriptor*> descriptor, Pipeline* pipeline, uint32_t firstDescriptor)
	{

		std::vector<VkDescriptorSet> sets;

		for (auto& x : descriptor)
		{
			if (x->m_Sets.size() == 0)
				continue;
			sets.push_back(x->m_Sets[getIndex()].set);
		}

		if (sets.size() == 0)
			return;

		vkCmdBindDescriptorSets(m_Cmd[getIndex()], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->m_Layout,
			firstDescriptor, sets.size(), sets.data(), 0, nullptr);
	}

	void CommandList::BindDescriptors(std::vector<Descriptor*> descriptor, ComputePipeline* pipeline, uint32_t firstDescriptor)
	{
		std::vector<VkDescriptorSet> sets;

		for (auto& x : descriptor)
		{
			sets.push_back(x->m_Sets[getIndex()].set);
		}

		vkCmdBindDescriptorSets(m_Cmd[getIndex()], VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->m_Layout,
			firstDescriptor, sets.size(), sets.data(), 0, nullptr);
	}

	void CommandList::PushConstants(vk::Pipeline* pipeline, ShaderStage stage, uint32_t size, uint32_t offset, void* data)
	{
		VkShaderStageFlags shaderStage = (VkShaderStageFlags)stage;

		vkCmdPushConstants(m_Cmd[getIndex()], pipeline->m_Layout, shaderStage, offset, size, data);
	}

	void CommandList::PushConstants(vk::ComputePipeline* pipeline, ShaderStage stage, uint32_t size, uint32_t offset, void* data)
	{
		VkShaderStageFlags shaderStage = (VkShaderStageFlags)stage;

		vkCmdPushConstants(m_Cmd[getIndex()], pipeline->m_Layout, shaderStage, offset, size, data);
	}

	void CommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
	{
		vkCmdDispatch(m_Cmd[getIndex()], x, y, z);
	}

	void CommandList::ImageBarrier(Texture* texture, PipelineStage srcStage, PipelineStage dstStage, ImageBarrierInfo barrierInfo)
	{

		VkImageMemoryBarrier imgMemBarrier{};
		imgMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgMemBarrier.srcAccessMask = (VkAccessFlags)barrierInfo.srcAccess;
		imgMemBarrier.dstAccessMask = (VkAccessFlags)barrierInfo.dstAccess;
		imgMemBarrier.image = texture->m_Image;
		imgMemBarrier.subresourceRange = texture->m_ResourceRange;
		imgMemBarrier.oldLayout = (VkImageLayout)barrierInfo.oldLayout;
		imgMemBarrier.newLayout = (VkImageLayout)barrierInfo.newLayout;

		vkCmdPipelineBarrier(m_Cmd[getIndex()], (VkPipelineStageFlags)srcStage, (VkPipelineStageFlags)dstStage, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &imgMemBarrier);
	}

	void CommandList::DrawIndexedIndirect(Buffer* buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
	{
		vkCmdDrawIndexedIndirect(m_Cmd[getIndex()], buffer->m_Buffer, offset, drawCount, stride);
	}

	void CommandList::CopyImage(Texture* src, ImageLayout srcLayout, Texture* dst, ImageLayout dstLayout, ImageCopy* region)
	{
		VkImageCopy copy;
		copy.srcOffset = { region->srcX, region->srcY };
		copy.dstOffset = { region->dstX, region->dstY };
		copy.extent = { region->w, region->h, 1 };
		copy.srcSubresource.mipLevel = 0;
		copy.srcSubresource.layerCount = 1;
		copy.srcSubresource.baseArrayLayer = region->srcLayer;
		copy.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy.dstSubresource.mipLevel = 0;
		copy.dstSubresource.layerCount = 1;
		copy.dstSubresource.baseArrayLayer = region->dstLayer;
		copy.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		vkCmdCopyImage(m_Cmd[getIndex()], src->m_Image, (VkImageLayout)srcLayout, dst->m_Image, (VkImageLayout)dstLayout, 1, &copy);
	}

	void CommandList::ClearColourImage(Texture* src, ImageLayout layout, glm::vec4 colour)
	{
		VkImageSubresourceRange range;
		range.baseMipLevel = 0;
		range.levelCount = src->m_MipLevels;
		range.layerCount = 1;
		range.baseArrayLayer = 0;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		VkClearColorValue val = { colour.r, colour.g, colour.b, colour.a };

		vkCmdClearColorImage(m_Cmd[getIndex()], src->m_Image, (VkImageLayout)layout, &val, 1, &range);
	}

	void CommandList::ClearDepth(float depthColour, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
	{
		VkClearAttachment attachment{};
		attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		attachment.clearValue = { depthColour, 0.0f };

		VkClearRect rect{};
		rect.baseArrayLayer = 0;
		rect.layerCount = 1;
		rect.rect.extent = { w, h };
		rect.rect.offset = { (int32_t)x, (int32_t)y };

		vkCmdClearAttachments(m_Cmd[getIndex()], 1, &attachment, 1, &rect);
	}

	void CommandList::SetLineWidth(float width)
	{
		vkCmdSetLineWidth(m_Cmd[getIndex()], width);
	}

	void CommandList::BeginDebugUtilsLabel(std::string name)
	{
		VkDebugUtilsLabelEXT label = {};
		label.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
		label.pNext = nullptr;
		label.pLabelName = name.c_str();

		m_Device->vkCmdBeginDebugUtilsLabel(m_Cmd[getIndex()], &label);
	}

	void CommandList::EndDebugUtilsLabel()
	{
		m_Device->vkCmdEndDebugUtilsLabel(m_Cmd[getIndex()]);
	}

	VkCommandBuffer CommandList::GetCommandBuffer()
	{
		return m_Cmd[getIndex()];
	}

	uint32_t CommandList::getIndex()
	{
		return m_Device->m_CurrentFrame;
	}
}