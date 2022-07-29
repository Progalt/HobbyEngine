#pragma once

#include <vulkan/vulkan.h>

#include <vector>

#include <glm/glm.hpp>

#include "Texture.h"

namespace vk
{
	class Device;
	class CommandList;
	class Pipeline;
	class Imgui;

	struct AttachmentDesc
	{
		VkFormat format;
		bool depth;
		VkImageLayout finalLayout;
		bool load = false;
		bool store = true;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	};

	enum class RenderpassType
	{
		Offscreen,
		Swapchain
	};

	

	struct RenderpassCreateInfo
	{
		glm::vec4 clearColour;
		float depthClear = 0.0f;
		RenderpassType type;

		std::vector<Texture*> colourAttachments;
		ImageLayout colourAttachmentInitialLayouts = ImageLayout::Undefined;
		ImageLayout colourAttachmentFinalLayouts = ImageLayout::ShaderReadOnlyOptimal;
		bool loadAttachments = false;
		Texture* depthAttachment;
		bool loadDepth = false;
		bool storeDepth = true;
		ImageLayout depthInitialLayout = ImageLayout::Undefined;
		
		uint32_t extentWidth = 0;
		uint32_t extentHeight = 0;
	};

	class Renderpass
	{
	public:

		void create(VkDevice device, VkFormat format, std::vector<AttachmentDesc> descs);

		void Destroy();

	private:

		friend Device;
		friend CommandList;
		friend Pipeline;
		friend Imgui;

		bool hasDepth;
		uint32_t m_NumTargets = 0;

		ImageLayout m_FinalLayout;
		std::vector<Texture*> m_ColourAttachments;

		glm::vec4 clearColour;

		float depthClearColour;

		RenderpassType type;

		VkDevice m_Device;

		VkRenderPass m_Renderpass;

		std::vector<VkFramebuffer> m_Framebuffers;

	};
}