#pragma once
#define VK_DONT_USE_BOOTSTRAP

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
#include <set>
#include <mutex>

#include "Vendor/VkBootstrap.h"

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

		bool requestVSync; 
	};

	struct SupportedFeatures
	{
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

	// Simple class to use for single use command buffer internally
	// It stores the thread the command buffer was allocated for
	class SingleUseCommandBuffer
	{
	public:

		VkCommandBuffer buffer; 
		uint32_t threadNum = 0;

		operator VkCommandBuffer()
		{
			return buffer;
		}
	};

	// Very very simple ref counter for threads
	// Everytime a command list is requested for a certain thread it increments and when its submitted it decrements
	// This means if a command list needs to be on a new thread it can request an empty thread
	class ThreadRefCounter
	{
	public:

		void Increment()
		{
			refs++;
		}

		void Decrement()
		{
			if (refs > 0)
				refs--;
		}

		bool empty()
		{
			return refs == 0;
		}

	private:


		uint32_t threadNum = 0;

		uint32_t refs = 0;
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
#ifdef VK_DONT_USE_BOOTSTRAP
		Swapchain* GetSwapchain() { return &m_Swapchain; }
#else 
		vkb::Swapchain GetSwapchain() { return m_Swapchain; }
#endif

		void ResizeSwapchain(uint32_t width, uint32_t height);

		// Returns the next available thread not being used by a command pool
		uint32_t GetThreadPoolNotInUse();

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

		// the single use command buffer system needs an overhaul

		SingleUseCommandBuffer GetSingleUsageCommandBuffer(bool transfer);

		void ExecuteTransfer(SingleUseCommandBuffer cmd, bool deferSubmission = false);

		void LoadExtensionFunctions();

		/* Handles */

#ifdef VK_DONT_USE_BOOTSTRAP
		VkInstance m_Instance;
		VkPhysicalDevice m_PhysicalDevice;

		VkDevice m_Device;

		Swapchain m_Swapchain;

#else 

		vkb::Instance m_Instance;
		vkb::PhysicalDevice m_PhysicalDevice;
		vkb::Device m_Device;

		vkb::Swapchain m_Swapchain;

#endif

		VkSurfaceKHR m_Surface;

		uint32_t m_GraphicsQueueFamily, m_PresentQueueFamily, m_ComputeQueueFamily, m_TransferQueueFamily;

	

		VkQueue m_GraphicsQueue;
		VkQueue m_PresentQueue;

		VkQueue m_TransferQueue;

		VmaAllocator m_Allocator;

		

		DescriptorAllocator m_DescriptorAllocator;

		std::vector<VkCommandPool> m_CommandPools;

		std::vector<VkSemaphore> m_ImageAvailable;
		std::vector<VkSemaphore> m_RenderFinished;
		std::vector<VkFence> m_InFlightFences;
		std::vector<VkFence> m_ImagesInFlight;

		std::vector<ThreadRefCounter> m_ThreadStates;

		uint32_t m_ImageIndex = 0;

		uint32_t m_CurrentFrame = 0;
		bool m_FirstFrame = true;

		PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel;
		PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel;

		bool m_ValidFrame = true;

		bool m_RequestSRGBBackBuffer;
		bool m_RequestVSync;

	};
}