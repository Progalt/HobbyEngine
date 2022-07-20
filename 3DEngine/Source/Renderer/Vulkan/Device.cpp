#include "Device.h"

#include <iostream>

#include <map>
#include <set>

namespace vk
{
	/* Public functions */

	void Device::Create(DeviceCreateInfo* info)
	{
		this->m_CreateInfo = info;
		this->m_RequestSRGBBackBuffer = info->requestSRGBBackBuffer;

		CreateInstance();

		SelectPhysicalDevice();

		CreateSurface();

		FindQueueFamilies();

		CreateLogicalDevice();

		CreateAllocator();

		if (!m_Swapchain.isSwapchainAvailable(m_PhysicalDevice, m_Surface))
			throw std::runtime_error("Swapchain not avaliable");

		m_Swapchain.createSwapchain(m_Device, m_CreateInfo->width, m_CreateInfo->height, m_Surface, m_GraphicsQueueFamily, m_PresentQueueFamily, m_RequestSRGBBackBuffer);

		CreateCommandPools();

		CreateSyncObjects();

		m_DescriptorAllocator.Init(m_Device);
	}

	void Device::ResizeSwapchain(uint32_t width, uint32_t height)
	{
		

		m_Swapchain.destroy(m_Device);

		if (!m_Swapchain.isSwapchainAvailable(m_PhysicalDevice, m_Surface))
			throw std::runtime_error("Swapchain not avaliable");

		m_Swapchain.createSwapchain(m_Device, width, height, m_Surface, m_GraphicsQueueFamily, m_PresentQueueFamily, m_RequestSRGBBackBuffer);

	}

	void Device::WaitIdle()
	{
		vkQueueWaitIdle(m_GraphicsQueue);
	}

	void Device::Destroy()
	{


		m_DescriptorAllocator.Cleanup();

		m_Swapchain.destroy(m_Device);



		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_Device, m_RenderFinished[i], nullptr);
			vkDestroySemaphore(m_Device, m_ImageAvailable[i], nullptr);
			vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
		}


		for (auto& cmd : m_CommandPools)
			vkDestroyCommandPool(m_Device, cmd, nullptr);



		vmaDestroyAllocator(m_Allocator);

		vkDestroyDevice(m_Device, nullptr);

		vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

		vkDestroyInstance(m_Instance, nullptr);
	}

	bool Device::NextFrame()
	{
		vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imageIndex;
		VkResult result = vkAcquireNextImageKHR(m_Device, m_Swapchain.m_Swapchain, UINT64_MAX, m_ImageAvailable[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

		if (result != VK_SUCCESS)
		{
			m_ValidFrame = false;
			return false;
		}

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			m_ValidFrame = false;
			return false;
		}

		m_ImageIndex = imageIndex;

		m_ValidFrame = true;
		return true;
	}

	void Device::SubmitCommandListsAndPresent(std::vector<CommandList> cmdList)
	{
		if (m_ValidFrame == false)
			return;

		if (m_ImagesInFlight[m_ImageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(m_Device, 1, &m_ImagesInFlight[m_ImageIndex], VK_TRUE, UINT64_MAX);
		}
		m_ImagesInFlight[m_ImageIndex] = m_InFlightFences[m_CurrentFrame];


		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailable[m_CurrentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		std::vector<VkCommandBuffer> buffers(cmdList.size());

		int i = 0;
		for (auto& cmd : cmdList)
		{

			buffers[i] = cmd.m_Cmd[m_CurrentFrame];
			i++;
		}

		submitInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
		submitInfo.pCommandBuffers = buffers.data();

		VkSemaphore signalSemaphores[] = { m_RenderFinished[m_CurrentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;


		vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

		if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit command lists to queue");
		}

		// Present

		VkSemaphore waitpresentSemaphores[] = { m_RenderFinished[m_CurrentFrame] };

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = waitpresentSemaphores;

		VkSwapchainKHR swapChains[] = { m_Swapchain.getSwapchain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &m_ImageIndex;
		presentInfo.pResults = nullptr;

		VkResult presentResult = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR)
		{
		}

		m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	}

	ShaderBlob Device::NewShaderBlob()
	{
		ShaderBlob blob;
		blob.m_Device = m_Device;

		return blob;
	}

	Pipeline Device::NewPipeline(PipelineCreateInfo* createInfo)
	{
		Pipeline p;
		p.Create(m_Device, createInfo);

		return p;
	}

	ComputePipeline Device::NewComputePipeline(ComputePipelineCreateInfo* createInfo)
	{
		ComputePipeline p;
		p.Create(m_Device, createInfo);

		return p;
	}

	Renderpass Device::NewRenderpass(RenderpassCreateInfo* createInfo)
	{
		Renderpass renderpass;

		renderpass.clearColour = createInfo->clearColour;
		renderpass.type = createInfo->type;
		renderpass.depthClearColour = createInfo->depthClear;

		uint32_t w = (createInfo->extentWidth == 0) ? m_Swapchain.getWidth() : createInfo->extentWidth;
		uint32_t h = (createInfo->extentHeight == 0) ? m_Swapchain.getHeight() : createInfo->extentHeight;

		//uint32_t w = createInfo->extentWidth;
		//uint32_t h = createInfo->extentHeight;

		if (w == 0 || h == 0)
			throw std::runtime_error("Can't create Framebuffer of width or height 0");

		if (createInfo->type == RenderpassType::Swapchain)
		{

			std::vector<AttachmentDesc> descs =
			{
				{ m_Swapchain.getFormat(), false, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR }
			};

			renderpass.create(m_Device, m_Swapchain.getFormat(), descs);

			renderpass.m_Framebuffers.resize(m_Swapchain.m_ImageViews.size());

			for (int i = 0; i < m_Swapchain.m_ImageViews.size(); i++)
			{
				VkImageView attachments[] = { m_Swapchain.m_ImageViews[i] };

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderpass.m_Renderpass;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = w;
				framebufferInfo.height = h;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &renderpass.m_Framebuffers[i]) != VK_SUCCESS)
				{
					throw std::exception("Failed to create framebuffer");
				}
			}

		}
		else if (createInfo->type == RenderpassType::Offscreen)
		{
			std::vector<AttachmentDesc> descs;
			std::vector<VkImageView> attachments;

			for (auto& a : createInfo->colourAttachments)
			{
				descs.push_back({ (VkFormat)a->format, false, (VkImageLayout)createInfo->colourAttachmentFinalLayouts, createInfo->loadAttachments, true, (VkImageLayout)createInfo->colourAttachmentInitialLayouts });

				attachments.push_back(a->m_ImageView);
			}

			if (createInfo->depthAttachment != nullptr)
			{
				descs.push_back({ (VkFormat)createInfo->depthAttachment->format, true, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, createInfo->loadDepth, createInfo->storeDepth, (VkImageLayout)createInfo->depthInitialLayout });

				attachments.push_back(createInfo->depthAttachment->m_ImageView);
			}

			renderpass.create(m_Device, VK_FORMAT_UNDEFINED, descs);

			renderpass.m_Framebuffers.resize(1);

			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderpass.m_Renderpass;
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = w;
			framebufferInfo.height = h;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &renderpass.m_Framebuffers[0]) != VK_SUCCESS)
			{
				throw std::exception("Failed to create framebuffer");
			}
		}

		return renderpass;
	}

	CommandList Device::NewCommandList(CommandListType type, uint32_t threadNum)
	{
		CommandList cmd;

		VkCommandBufferLevel level;
		switch (type)
		{
		case CommandListType::Primary: level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; break;
		case CommandListType::Secondary: level = VK_COMMAND_BUFFER_LEVEL_SECONDARY; break;
		default: level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; break;
		}

		cmd.type = type;


		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_CommandPools[threadNum];
		allocInfo.level = level;
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_Swapchain.m_ImageViews.size());

		cmd.threadNum = threadNum;
		cmd.m_Secondary = (type == CommandListType::Secondary) ? true : false;

		cmd.m_Cmd.resize(m_Swapchain.m_ImageViews.size());

		if (vkAllocateCommandBuffers(m_Device, &allocInfo, cmd.m_Cmd.data()) != VK_SUCCESS)
		{
			throw std::exception("Failed to allocate command list");
		}

		cmd.m_Device = this;

		return cmd;
	}

	Buffer Device::NewBuffer()
	{
		Buffer buf;
		buf.m_Allocator = m_Allocator;
		buf.m_Device = this;

		return buf;
	}

	Texture Device::NewTexture()
	{
		Texture tex;
		tex.m_Allocator = m_Allocator;
		tex.m_Device = this;

		return tex;
	}

	Sampler Device::NewSampler(SamplerSettings* settings)
	{
		Sampler sampler;
		sampler.Create(settings, m_Device);
		
		return sampler;
	}

	Descriptor Device::NewDescriptor(DescriptorLayout* layout)
	{
		Descriptor desc;

		desc.m_Device = m_Device;
		desc.m_GraphicsDevice = this;

		for (int i = 0; i < m_Swapchain.getImageCount(); i++)
		{
			VkDescriptorSet set;

			m_DescriptorAllocator.Allocate(&set, layout->m_SetLayout);

			desc.m_Sets.push_back({ {}, { set } });

		}

		return desc;
	}

	DescriptorLayout Device::NewLayout()
	{
		DescriptorLayout layout;
		layout.m_Device = m_Device;
		return layout;
	}

	/* Private Functions */

	bool checkLayerSupport(std::vector<const char*> wantedLayers)
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : wantedLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
			{
				return false;
			}
		}

		return true;
	}

	void Device::CreateInstance()
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Kara App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Kara GFX";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_3;

		std::vector<const char*> extensions;

		/* Windows only support right now */

		extensions.push_back("VK_KHR_surface");
		extensions.push_back("VK_KHR_win32_surface");
		extensions.push_back("VK_EXT_debug_utils");

		std::vector<const char*> wantedLayers = {};

		if (m_CreateInfo->debugInfo)
		{
			wantedLayers.push_back("VK_LAYER_KHRONOS_validation");
		}

		if (!checkLayerSupport(wantedLayers))
			throw std::runtime_error("Specified Instance layers are not avaliable");

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(wantedLayers.size());
		createInfo.ppEnabledLayerNames = wantedLayers.data();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Instance");



	}

	int rateDeviceSuitability(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		int score = 0;

		/*
		* Discrete GPUs usually are the best over integrated
		* This assumes this is the case
		*/

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		// Select best GPU based on the engines needs

		score += deviceProperties.limits.maxDescriptorSetSampledImages;
		score += deviceProperties.limits.maxDescriptorSetSamplers;
		score += deviceProperties.limits.maxBoundDescriptorSets;
		score += deviceProperties.limits.maxDrawIndexedIndexValue;
		score += deviceProperties.limits.maxImageDimension2D;

		return score;
	}


	void Device::SelectPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		if (devices.size() == 0)
		{
			throw std::runtime_error("No GPUs that support vulkan");
		}

		std::multimap<int, VkPhysicalDevice> candidates;

		for (const auto& device : devices)
		{
			int score = rateDeviceSuitability(device);
			candidates.insert(std::make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0)
		{
			m_PhysicalDevice = candidates.rbegin()->second;

			VkPhysicalDeviceProperties deviceProperties;
			vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);

			std::cout << "Selected GPU: " + std::string(deviceProperties.deviceName) << std::endl;
			m_DeviceInfo.deviceName = deviceProperties.deviceName;
			switch (deviceProperties.vendorID)
			{
			case 0x1002: m_DeviceInfo.vendorName = "AMD"; break;
			case 0x1010: m_DeviceInfo.vendorName = "ImgTec"; break;
			case 0x10DE: m_DeviceInfo.vendorName = "NVIDIA"; break;
			case 0x13B5: m_DeviceInfo.vendorName = "ARM"; break;
			case 0x5143: m_DeviceInfo.vendorName = "Qualcomm"; break;
			case 0x8086: m_DeviceInfo.vendorName = "INTEL"; break;
			}

		}
		else
		{
			throw std::runtime_error("Failed to find suitable GPU");
		}
	}

	void Device::CreateSurface()
	{
		/*VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = m_CreateInfo->window->GetRawWindow();
		createInfo.hinstance = GetModuleHandle(nullptr);

		if (vkCreateWin32SurfaceKHR(m_Instance, &createInfo, nullptr, &m_Surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Win32 Surface");
		}*/

		m_Surface = m_CreateInfo->window->CreateSurface(m_Instance);

	}

	void Device::FindQueueFamilies()
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

		if (queueFamilyCount == 0)
		{
			throw std::runtime_error("No Queue families found");
		}
		else
		{
			std::cout << "Device Queue Family Count: " << queueFamilyCount << std::endl;
		}


		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		bool foundGraphicsQueueFamily = false;
		bool foundPresentQueueFamily = false;
		bool foundTransferQueueFamily = false;
		bool foundComputeQueueFamily = false;

		for (uint32_t i = 0; i < queueFamilyCount; i++)
		{


			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &presentSupport);

			std::cout << "Queue Family " << i << " Supports: " << ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "Graphics, " : "") <<
				((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) ? "Compute, " : "") <<
				((queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) ? "Transfer, " : "") <<
				((presentSupport) ? "Present " : "") << std::endl;

			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && !foundTransferQueueFamily)
			{

				m_GraphicsQueueFamily = i;
				foundGraphicsQueueFamily = true;
			}

			if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT && !foundComputeQueueFamily)
			{

				m_ComputeQueueFamily = i;
				foundComputeQueueFamily = true;
			}

			if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && !foundTransferQueueFamily)
			{
				m_TransferQueueFamily = i;
				foundTransferQueueFamily = true;
			}
			else if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT && m_TransferQueueFamily == m_GraphicsQueueFamily)
			{
				// Try to grab a dedicated transfer queue 
				m_TransferQueueFamily = i;
			}

			if (presentSupport && !foundPresentQueueFamily)
			{
				m_PresentQueueFamily = i;
				foundPresentQueueFamily = true;
			}

		}

		if (foundGraphicsQueueFamily)
		{

			if (foundPresentQueueFamily)
			{
			}
			else
			{
				throw std::runtime_error("Failed to find Queue family that supports present");
			}
		}
		else
		{

			throw std::runtime_error("Failed to find Queue family that supports graphics");
		}
	}

	bool checkDeviceExtensionSupport(std::vector<const char*> wantedExtensions, VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());


		std::set<std::string> requiredExtensions(wantedExtensions.begin(), wantedExtensions.end());

		//std::cout << "Available Device Extensions: \n";
		for (const auto& extension : availableExtensions)
		{
			//std::cout << " - " << extension.extensionName << std::endl;

			requiredExtensions.erase(extension.extensionName);
		}


		return requiredExtensions.empty();
	}

	void Device::CreateLogicalDevice()
	{
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies;

		uniqueQueueFamilies = { m_GraphicsQueueFamily, m_PresentQueueFamily, m_ComputeQueueFamily, m_TransferQueueFamily };

		std::cout << "Unique Queue Family Size: " << uniqueQueueFamilies.size() << "\n";

		float queuePriority = 1.0f;


		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR supportedRaytracing{};
		supportedRaytracing.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;

		VkPhysicalDeviceDescriptorIndexingFeatures supportedIndexingFeatures{};
		supportedIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		supportedIndexingFeatures.pNext = &supportedRaytracing;

		VkPhysicalDeviceFeatures2 supportedFeatures{};
		supportedFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		supportedFeatures.pNext = &supportedIndexingFeatures;
		
		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &supportedFeatures);

		m_DeviceInfo.supportedFeatures.multiDrawIndirect = supportedFeatures.features.multiDrawIndirect;
		m_DeviceInfo.supportedFeatures.descriptorBindingPartiallyBound = supportedIndexingFeatures.descriptorBindingPartiallyBound;
		m_DeviceInfo.supportedFeatures.runtimeDescriptorArray = supportedIndexingFeatures.runtimeDescriptorArray;
		m_DeviceInfo.supportedFeatures.raytracing = supportedRaytracing.rayTracingPipeline;
		
		
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR raytracingFeature{};
		raytracingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		raytracingFeature.rayTracingPipeline = (VkBool32)m_DeviceInfo.supportedFeatures.raytracing;
		
		VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
		indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexingFeatures.pNext = &raytracingFeature;
		indexingFeatures.descriptorBindingPartiallyBound = (VkBool32)m_DeviceInfo.supportedFeatures.descriptorBindingPartiallyBound;
		indexingFeatures.runtimeDescriptorArray = (VkBool32)m_DeviceInfo.supportedFeatures.runtimeDescriptorArray;

		VkPhysicalDeviceFeatures2 features{};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		features.pNext = &indexingFeatures;
		features.features.multiDrawIndirect = (VkBool32)m_DeviceInfo.supportedFeatures.multiDrawIndirect;

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

		deviceCreateInfo.pNext = &features;
		
		std::vector<const char*> deviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		if (!checkDeviceExtensionSupport(deviceExtensions, m_PhysicalDevice))
		{
			throw std::runtime_error("Device Extensions aren't supported");
		}

		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create Queue");
		}

		vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
		vkGetDeviceQueue(m_Device, m_PresentQueueFamily, 0, &m_PresentQueue);



		vkCmdBeginDebugUtilsLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(m_Device, "vkCmdBeginDebugUtilsLabelEXT");
		vkCmdEndDebugUtilsLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(m_Device, "vkCmdEndDebugUtilsLabelEXT");
	}
	

	void Device::CreateAllocator()
	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
		allocatorInfo.physicalDevice = m_PhysicalDevice;
		allocatorInfo.device = m_Device;
		allocatorInfo.instance = m_Instance;

		if (vmaCreateAllocator(&allocatorInfo, &m_Allocator) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Allocator");
	}

	void Device::CreateCommandPools()
	{
		m_CommandPools.resize(m_CreateInfo->threadCount);

		for (uint32_t i = 0; i < m_CreateInfo->threadCount; i++)
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = m_GraphicsQueueFamily;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

			if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPools[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create command pools");
			}
		}

	}

	void Device::CreateSyncObjects()
	{
		m_ImageAvailable.resize(MAX_FRAMES_IN_FLIGHT);
		m_RenderFinished.resize(MAX_FRAMES_IN_FLIGHT);
		m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_ImagesInFlight.resize(m_Swapchain.getImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailable[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinished[i]) != VK_SUCCESS ||
				vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
			{

				throw std::runtime_error("Failed to create Sync objects");
			}
		}

	}

	VkCommandBuffer Device::GetSingleUsageCommandBuffer(bool transfer)
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		//allocInfo.commandPool = (transfer) ? m_TransferPools[m_TransferPools.size() - 1] : m_CommandPools[m_CommandPool.size() - 1];

		allocInfo.commandPool = m_CommandPools[m_CommandPools.size() - 1];

		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_Device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		return commandBuffer;
	}

	void Device::ExecuteTransfer(VkCommandBuffer cmd)
	{
		vkEndCommandBuffer(cmd);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_GraphicsQueue);
		vkFreeCommandBuffers(m_Device, m_CommandPools[m_CommandPools.size() - 1], 1, &cmd);
	}
}