#include "Swapchain.h"

#include <algorithm>
#include <iostream>

namespace vk
{
	void Swapchain::destroy(VkDevice device)
	{
		for (auto imageView : m_ImageViews)
			vkDestroyImageView(device, imageView, nullptr);

		vkDestroySwapchainKHR(device, m_Swapchain, nullptr);
	}

	void Swapchain::createSwapchain(VkDevice device, uint32_t width, uint32_t height, VkSurfaceKHR surface, uint32_t graphicsQueueFamily, uint32_t presentQueueFamily, bool requestSRGB)
	{


		VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(requestSRGB);
		VkPresentModeKHR presentMode = choosePresentMode();
		m_Extent = chooseExtent(width, height);

		uint32_t imageCount = m_Details.capabilities.minImageCount + 1;

		if (m_Details.capabilities.maxImageCount > 0 && imageCount > m_Details.capabilities.maxImageCount)
		{
			imageCount = m_Details.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = m_Extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { graphicsQueueFamily, presentQueueFamily };

		if (graphicsQueueFamily != presentQueueFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}


		createInfo.preTransform = m_Details.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;


		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Swapchain");
		}


		// Retrieve Images

		uint32_t images;
		vkGetSwapchainImagesKHR(device, m_Swapchain, &images, nullptr);
		m_Images.resize(images);
		vkGetSwapchainImagesKHR(device, m_Swapchain, &images, m_Images.data());

		m_Format = surfaceFormat.format;

		m_ImageCount = images;

		// Create image views

		m_ImageViews.resize(m_Images.size());

		for (size_t i = 0; i < m_Images.size(); i++)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_Images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_Format;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &createInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create image view");
			}
		}

	}



	bool Swapchain::isSwapchainAvailable(VkPhysicalDevice device, VkSurfaceKHR surface)
	{

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &m_Details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			m_Details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, m_Details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			m_Details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, m_Details.presentModes.data());
		}

		if (m_Details.formats.empty() && m_Details.presentModes.empty())
		{

			throw std::runtime_error("Swapchain not supported");

			return false;
		}
		else if (!m_Details.formats.empty() && !m_Details.presentModes.empty())
		{
		}

		return !m_Details.formats.empty() && !m_Details.presentModes.empty();
	}



	VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(bool requestSRGB)
	{
		for (const auto& availableFormat : m_Details.formats)
		{

			if (requestSRGB)
			{
				// Get an SRGB image if the format is available
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				{
					return availableFormat;
				}
			}
		}

		return m_Details.formats[0];
	}

	VkPresentModeKHR Swapchain::choosePresentMode()
	{
		//std::cout << "Avaliable Present Modes: \n";
		/*for (const auto& availablePresentMode : m_Details.presentModes)
		{
			switch (availablePresentMode)
			{
			case VK_PRESENT_MODE_IMMEDIATE_KHR: std::cout << " - VK_PRESENT_MODE_IMMEDIATE_KHR\n"; break;
			case VK_PRESENT_MODE_MAILBOX_KHR: std::cout << " - VK_PRESENT_MODE_MAILBOX_KHR\n"; break;
			case VK_PRESENT_MODE_FIFO_KHR: std::cout << " - VK_PRESENT_MODE_FIFO_KHR\n"; break;
			case VK_PRESENT_MODE_FIFO_RELAXED_KHR: std::cout << " - VK_PRESENT_MODE_FIFO_RELAXED_KHR \n"; break;
			}
		}*/

		VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;

		bool immediateSupport = false;
		bool mailboxSupport = false;

		for (const auto& availablePresentMode : m_Details.presentModes)
		{
			switch (availablePresentMode)
			{
			case VK_PRESENT_MODE_IMMEDIATE_KHR: immediateSupport = true; break;
			case VK_PRESENT_MODE_MAILBOX_KHR: mailboxSupport = true; break;
			}
		}

		if (mailboxSupport && !immediateSupport)
		{
			std::cout << "Selected Present Mode: Mailbox\n";
			mode = VK_PRESENT_MODE_MAILBOX_KHR;
		}
		else if (immediateSupport)
		{
			std::cout << "Selected Present Mode: Immediate\n";
			mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
		else
		{
			std::cout << "Selected Present Mode: Fifo\n";
		}

		// Spec requires this to be available
		// this is vertical sync
		return mode;
	}

	VkExtent2D Swapchain::chooseExtent(uint32_t width, uint32_t height)
	{
		if (m_Details.capabilities.currentExtent.width != UINT32_MAX)
		{
			return m_Details.capabilities.currentExtent;
		}
		else
		{

			VkExtent2D actualExtent =
			{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, m_Details.capabilities.minImageExtent.width, m_Details.capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, m_Details.capabilities.minImageExtent.height, m_Details.capabilities.maxImageExtent.height);


			return actualExtent;
		}
	}
}