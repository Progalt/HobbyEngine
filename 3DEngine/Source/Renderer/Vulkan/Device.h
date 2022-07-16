#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "Vendor/vk_mem_alloc.h"

#include "../../Core/Window.h"

#include "Swapchain.h"

#include "CommandList.h"

#include "Pipeline.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "Descriptor.h"
#include "ComputePipeline.h"

#include "DescriptorAllocator.h"

#include "ImguiImpl.h"

#include <functional>
#include <stdexcept>

namespace vk
{
	struct DeviceCreateInfo
	{
		Window* window;
		bool debugInfo;
		uint32_t threadCount = 1;
		uint32_t width, height;
		
		// Excplicity request an SRGB BackBuffer
		// Does not gurantee one and also if this is false does not gurantee the selected backbuffer is not SRGB
		bool requestSRGBBackBuffer;
	};

	struct SupportedFeatures
	{
		bool raytracing = false;
		bool multiDrawIndirect = false;
		bool descriptorBindingPartiallyBound = false;
		bool runtimeDescriptorArray = false;
		
	};

	struct DeviceInfo
	{
		std::string vendorName;
		std::string deviceName;

		SupportedFeatures supportedFeatures;
	};

	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;

	class Device
	{
	public:

		void Create(DeviceCreateInfo* info);

		void Destroy();

		bool NextFrame();

		void WaitIdle();

		ShaderBlob NewShaderBlob();

		Pipeline NewPipeline(PipelineCreateInfo* createInfo);

		ComputePipeline NewComputePipeline(ComputePipelineCreateInfo* createInfo);

		Renderpass NewRenderpass(RenderpassCreateInfo* createInfo);

		CommandList NewCommandList(CommandListType type, uint32_t threadNum = 0);

		Buffer NewBuffer();

		Texture NewTexture();

		Sampler NewSampler(SamplerSettings* settings);

		DescriptorLayout NewLayout();

		Descriptor NewDescriptor(DescriptorLayout* layout);

		void SubmitCommandListsAndPresent(std::vector<CommandList> cmdList);

		DeviceInfo GetDeviceInfo() const { return m_DeviceInfo; }

		Swapchain* GetSwapchain() { return &m_Swapchain; }

		void ResizeSwapchain(uint32_t width, uint32_t height);


		/* Vulkan Handles */

		VkDevice GetDevice() { return m_Device; }

		VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }

	private:

		friend CommandList;
		friend Buffer;
		friend Texture;
		friend Descriptor;
		friend Imgui;

		DeviceCreateInfo* m_CreateInfo;		// lifetime only guaranteed during creation 

		DeviceInfo m_DeviceInfo;

		/* Create functions */

		void CreateInstance();

		void SelectPhysicalDevice();

		void CreateSurface();

		void FindQueueFamilies();

		void CreateLogicalDevice();

		void CreateAllocator();

		void CreateCommandPools();

		void CreateSyncObjects();

		VkCommandBuffer GetSingleUsageCommandBuffer(bool transfer);

		void ExecuteTransfer(VkCommandBuffer cmd);

		/* Handles */

		VkInstance m_Instance;
		VkPhysicalDevice m_PhysicalDevice;

		VkSurfaceKHR m_Surface;

		uint32_t m_GraphicsQueueFamily, m_PresentQueueFamily, m_ComputeQueueFamily, m_TransferQueueFamily;

		VkDevice m_Device;

		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		VkQueue m_TransferQueue;

		VmaAllocator m_Allocator;

		Swapchain m_Swapchain;

		DescriptorAllocator m_DescriptorAllocator;

		std::vector<VkCommandPool> m_CommandPools;

		std::vector<VkSemaphore> m_ImageAvailable;
		std::vector<VkSemaphore> m_RenderFinished;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;

		uint32_t m_ImageIndex = 0;

		uint32_t m_CurrentFrame = 0;

		PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel;
		PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel;

		bool m_ValidFrame = true;

		bool m_RequestSRGBBackBuffer;

	};
}