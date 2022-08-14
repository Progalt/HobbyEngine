#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace vk
{
	class Device;

	class Swapchain
	{
	public:
		void destroy(VkDevice device);

		bool isSwapchainAvailable(VkPhysicalDevice device, VkSurfaceKHR surface);

		void createSwapchain(VkDevice device, uint32_t width, uint32_t height, VkSurfaceKHR surface, uint32_t graphicsQueueFamily, uint32_t presentQueueFamily, bool requestSRGB, bool requestVSync);

		uint32_t getImageCount() const { return m_ImageCount; }

		std::vector<VkImageView> getImageViews() { return m_ImageViews; }

		VkImageView getImageView(uint32_t index) { return m_ImageViews[index]; }

		VkFormat getFormat() const { return m_Format; }

		VkSwapchainKHR getSwapchain() { return m_Swapchain; }

		uint32_t getWidth() const { return m_Extent.width; }
		uint32_t getHeight() const { return m_Extent.height; }

		VkExtent2D getExtent() const { return m_Extent; }

	private:

		friend Device;

		/* Swapchain */

		VkSwapchainKHR m_Swapchain;
		VkExtent2D m_Extent;
		VkFormat m_Format;

		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;

		uint32_t m_ImageCount = 0;

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		} m_Details;


		/* Selection functions */

		VkSurfaceFormatKHR chooseSurfaceFormat(bool requestSRGB);

		VkPresentModeKHR choosePresentMode(bool vsync);

		VkExtent2D chooseExtent(uint32_t width, uint32_t height);
	};
}