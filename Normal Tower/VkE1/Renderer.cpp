#include "Renderer.h"

#include <assert.h>

#include "Engine.h"

#define GRAPHICS_PRESENT_QUEUE_INDEX 0

#define VIEW_PROJECTION_UNIFORM_BINDING 0
#define MODEL_MATRICES_UNIFORM_BINDING 1
#define POINT_LIGHT_UNIFORM_BINDING 2
#define TEXTURE_UNIFORM_BINDING 3

void Renderer::Init()
{
	/// Logger
	{
#if _DEBUG
		logger.Start("_RendererLogger.txt");
		debugReportCallbackLogger.Start("DebugReportCallbackLogger.txt");
		cleanUpLogger.Start("_RendererCleanUpLogger.txt");
#endif
	}

	/// Instance
	const char* appName = "AppName";
	uint32_t appVersion = 0;
	const char* engineName = "EngineName";
	uint32_t engineVersion = 0;
	std::vector<const char*> enabledInstanceLayerNames =
	{
		//"VK_LAYER_LUNARG_swapchain",
#if _DEBUG
		"VK_LAYER_LUNARG_standard_validation",
#endif
	};
	std::vector<const char*> enabledInstanceExtensionNames =
	{
		"VK_KHR_surface",
		"VK_KHR_win32_surface",
#if _DEBUG
		"VK_EXT_debug_report",
#endif
	};
	{
		VkApplicationInfo applicationInfo;
		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		applicationInfo.pNext = nullptr;
		applicationInfo.pApplicationName = appName;
		applicationInfo.applicationVersion = appVersion;
		applicationInfo.pEngineName = engineName;
		applicationInfo.engineVersion = engineVersion;
		applicationInfo.apiVersion = VK_API_VERSION_1_0;

		uint32_t propertyCount = 0;
		std::vector<VkLayerProperties> availableInstanceLayerNames;
		VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&propertyCount, nullptr), "????????????????", "vkEnumerateInstanceLayerProperties");
		availableInstanceLayerNames.resize(propertyCount);
		VK_CHECK_RESULT(vkEnumerateInstanceLayerProperties(&propertyCount, availableInstanceLayerNames.data()), "????????????????", "vkEnumerateInstanceLayerProperties");

		for (size_t i = 0; i != enabledInstanceLayerNames.size(); ++i)
		{
			bool found = false;
			for (size_t j = 0; j != availableInstanceLayerNames.size(); ++j)
			{
				if (strcmp(enabledInstanceLayerNames[i], availableInstanceLayerNames[j].layerName) == 0)
				{
					found = true;
					break;
				}
			}

#if _DEBUG
			if (found == false)
				logger << "ERROR: Instance Layer" << enabledInstanceLayerNames[i] << " is not available.\n";
#endif
		}

		propertyCount = 0;
		std::vector<VkExtensionProperties> availableInstanceExtensionNames;
		VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr), "????????????????", "vkEnumerateInstanceExtensionProperties");
		availableInstanceExtensionNames.resize(propertyCount);
		VK_CHECK_RESULT(vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, availableInstanceExtensionNames.data()), "????????????????", "vkEnumerateInstanceExtensionProperties");

		for (size_t i = 0; i != enabledInstanceExtensionNames.size(); ++i)
		{
			bool found = false;
			for (size_t j = 0; j != availableInstanceExtensionNames.size(); ++j)
			{
				if (strcmp(enabledInstanceExtensionNames[i], availableInstanceExtensionNames[j].extensionName) == 0)
				{
					found = true;
					break;
				}
			}

#if _DEBUG
			if (found == false)
				logger << "ERROR: Instance Extension" << enabledInstanceExtensionNames[i] << " is not available.\n";
#endif
		}

		VkInstanceCreateInfo instanceCreateInfo;
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pNext = nullptr;
		instanceCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;
		instanceCreateInfo.enabledLayerCount = (uint32_t)enabledInstanceLayerNames.size();
		instanceCreateInfo.ppEnabledLayerNames = enabledInstanceLayerNames.data();
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledInstanceExtensionNames.size();
		instanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames.data();

		VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &instance), instance, "vkCreateInstance");
	}

	/// Debug
	VkDebugReportFlagsEXT debugFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT;;;
	{
#if _DEBUG
		VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo;
		debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugReportCallbackCreateInfo.pNext = nullptr;
		debugReportCallbackCreateInfo.flags = debugFlags;
		debugReportCallbackCreateInfo.pfnCallback = VkU::DebugReportCallback;
		debugReportCallbackCreateInfo.pUserData = nullptr;

		PFN_vkCreateDebugReportCallbackEXT FP_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		VK_CHECK_RESULT(FP_vkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCreateInfo, nullptr, &debugReportCallback), debugReportCallback, "FP_vkCreateDebugReportCallbackEXT");
#else
		debugReportCallback = VK_NULL_HANDLE;
#endif
	}

	/// Physical Device
	{
		uint32_t propertyCount = 0;
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &propertyCount, nullptr), "????????????????", "vkEnumeratePhysicalDevices");
		std::vector<VkPhysicalDevice> physicalDevicesHandles(propertyCount);
		VK_CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &propertyCount, physicalDevicesHandles.data()), "????????????????", "vkEnumeratePhysicalDevices");

		physicalDevices.resize(physicalDevicesHandles.size());
		for (size_t i = 0; i != physicalDevicesHandles.size(); ++i)
		{
			physicalDevices[i].handle = physicalDevicesHandles[i];

			vkGetPhysicalDeviceProperties(physicalDevices[i].handle, &physicalDevices[i].properties);
			vkGetPhysicalDeviceFeatures(physicalDevices[i].handle, &physicalDevices[i].features);
			vkGetPhysicalDeviceMemoryProperties(physicalDevices[i].handle, &physicalDevices[i].memoryProperties);

			uint32_t propertyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i].handle, &propertyCount, nullptr);
			physicalDevices[i].queueFamilyProperties.resize(propertyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[i].handle, &propertyCount, physicalDevices[i].queueFamilyProperties.data());

			physicalDevices[i].queueFamilyPresentable.resize(physicalDevices[i].queueFamilyProperties.size());
			for (uint32_t j = 0; j != physicalDevices[i].queueFamilyPresentable.size(); ++j)
				physicalDevices[i].queueFamilyPresentable[j] = vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevices[i].handle, j);

			physicalDevices[i].depthFormat = VkU::GetDepthFormat(physicalDevices[i].handle, nullptr);
		}
	}

	/// OS Window
	int width = 800;
	int height = 600;
	const char* windowTitle = "Window Title";
	const char* windowName = "Window Name";
	WNDPROC wndProc = nullptr;
	{
		window = VkU::GetWindow(width, height, windowTitle, windowName, wndProc);
	}

	/// Surface
	{
		VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo;
		win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		win32SurfaceCreateInfo.pNext = nullptr;
		win32SurfaceCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		win32SurfaceCreateInfo.hinstance = window.hInstance;
		win32SurfaceCreateInfo.hwnd = window.hWnd;

		VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfo, nullptr, &surface.handle), surface.handle, "vkCreateWin32SurfaceKHR");
	}

	/// PhysicalDevice & Queue picking
	std::vector<VkU::Queue> queue = { VkU::Queue::GetQueue(VK_TRUE, VK_QUEUE_GRAPHICS_BIT, 1.0f, 1) };
	{
		device.physicalDeviceIndex = -1;

		for (size_t i = 0; i != physicalDevices.size(); ++i)
		{
			bool compatible = false;
			device.queues = VkU::PickDeviceQueuesIndices(queue, physicalDevices[i], { surface }, &compatible);

			if (compatible == true)
			{
				device.physicalDeviceIndex = (uint32_t)i;
				break;
			}
		}
	}

	/// Device
	VkPhysicalDeviceFeatures features = {};
	features.samplerAnisotropy = true;
	std::vector<const char*> enabledDeviceLayerNames =
	{
#if _DEBUG
		"VK_LAYER_LUNARG_standard_validation",
#endif
	};
	std::vector<const char*> enabledDeviceExtensionNames =
	{
		"VK_KHR_swapchain",
	};
	{
		std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos(queue.size());
		for (size_t i = 0; i != deviceQueueCreateInfos.size(); ++i)
		{
			deviceQueueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCreateInfos[i].pNext = nullptr;
			deviceQueueCreateInfos[i].flags = VK_RESERVED_FOR_FUTURE_USE;
			deviceQueueCreateInfos[i].queueFamilyIndex = device.queues[i].queueFamilyIndex;
			deviceQueueCreateInfos[i].queueCount = device.queues[i].count;
			deviceQueueCreateInfos[i].pQueuePriorities = &device.queues[i].priority;
		}

		uint32_t propertyCount = 0;
		std::vector<VkLayerProperties> availableDeviceLayerProperties;
		VK_CHECK_RESULT(vkEnumerateDeviceLayerProperties(physicalDevices[device.physicalDeviceIndex].handle, &propertyCount, nullptr), "????????????????", "vkEnumerateDeviceLayerProperties");
		availableDeviceLayerProperties.resize(propertyCount);
		VK_CHECK_RESULT(vkEnumerateDeviceLayerProperties(physicalDevices[device.physicalDeviceIndex].handle, &propertyCount, availableDeviceLayerProperties.data()), "????????????????", "vkEnumerateDeviceLayerProperties");

		for (size_t i = 0; i != enabledDeviceLayerNames.size(); ++i)
		{
			bool found = false;
			for (size_t j = 0; j != availableDeviceLayerProperties.size(); ++j)
			{
				if (strcmp(enabledDeviceLayerNames[i], availableDeviceLayerProperties[j].layerName) == 0)
				{
					found = true;
					break;
				}
			}

#if _DEBUG
			if (found == false)
				logger << "ERROR: Device Layer" << enabledDeviceLayerNames[i] << " is not available.\n";
#endif
		}

		propertyCount = 0;
		std::vector<VkExtensionProperties> availableDeviceExtensionProperties;
		VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevices[device.physicalDeviceIndex].handle, nullptr, &propertyCount, nullptr), "????????????????", "vkEnumerateDeviceExtensionProperties");
		availableDeviceExtensionProperties.resize(propertyCount);
		VK_CHECK_RESULT(vkEnumerateDeviceExtensionProperties(physicalDevices[device.physicalDeviceIndex].handle, nullptr, &propertyCount, availableDeviceExtensionProperties.data()), "????????????????", "vkEnumerateDeviceExtensionProperties");

		for (size_t i = 0; i != enabledDeviceExtensionNames.size(); ++i)
		{
			bool found = false;
			for (size_t j = 0; j != availableDeviceExtensionProperties.size(); ++j)
			{
				if (strcmp(enabledDeviceExtensionNames[i], availableDeviceExtensionProperties[j].extensionName) == 0)
				{
					found = true;
					break;
				}
			}

#if _DEBUG
			if (found == false)
				logger << "ERROR: Device Extension" << enabledDeviceExtensionNames[i] << " is not available.\n";
#endif
		}

		VkDeviceCreateInfo deviceCreateInfo;
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pNext = nullptr;
		deviceCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		deviceCreateInfo.queueCreateInfoCount = (uint32_t)deviceQueueCreateInfos.size();
		deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
		deviceCreateInfo.enabledLayerCount = (uint32_t)enabledDeviceLayerNames.size();
		deviceCreateInfo.ppEnabledLayerNames = enabledDeviceLayerNames.data();
		deviceCreateInfo.enabledExtensionCount = (uint32_t)enabledDeviceExtensionNames.size();
		deviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames.data();
		deviceCreateInfo.pEnabledFeatures = &features;

		VK_CHECK_RESULT(vkCreateDevice(physicalDevices[device.physicalDeviceIndex].handle, &deviceCreateInfo, nullptr, &device.handle), device.handle, "vkCreateDevice");
	}

	/// Queues
	{
		for (size_t i = 0; i != device.queues.size(); ++i)
		{
			device.queues[i].handles.resize(device.queues[i].count);
			for (uint32_t j = 0; j != device.queues[i].count; ++j)
			{
				vkGetDeviceQueue(device.handle, device.queues[i].queueFamilyIndex, device.queues[i].queueIndex + j, &device.queues[i].handles[j]);
			}
		}
	}

	/// Surface properties
	{
		surface.colorFormat = VkU::GetVkSurfaceFormatKHR(physicalDevices[device.physicalDeviceIndex].handle, surface, nullptr);
		surface.compositeAlpha = VkU::GetVkCompositeAlphaFlagBitsKHR(VkU::GetVkSurfaceCapabilitiesKHR(physicalDevices[device.physicalDeviceIndex].handle, surface.handle), &VkU::preferedCompositeAlphas);
		surface.presentMode = VkU::GetVkPresentModeKHR(physicalDevices[device.physicalDeviceIndex].handle, surface.handle, &VkU::preferedPresentModes);
	}

	/// CommandPool
	{
		VkCommandPoolCreateInfo commandPoolCreateInfo;
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolCreateInfo.pNext = nullptr;
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolCreateInfo.queueFamilyIndex = device.queues[GRAPHICS_PRESENT_QUEUE_INDEX].queueFamilyIndex;

		VK_CHECK_RESULT(vkCreateCommandPool(device.handle, &commandPoolCreateInfo, nullptr, &commandPool), commandPool, "vkCreateCommandPool");
	}

	/// Setup command buffer
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;

		VK_CHECK_RESULT(vkAllocateCommandBuffers(device.handle, &commandBufferAllocateInfo, &setupCommandBuffer), setupCommandBuffer, "vkAllocateCommandBuffers");
	}

	/// Setup fence
	{
		VkFenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VK_CHECK_RESULT(vkCreateFence(device.handle, &fenceCreateInfo, nullptr, &setupFence), setupFence, "vkCreateFence");
	}

	/// RenderPass
	{
		VkAttachmentDescription colorAttachmentDescription;
		colorAttachmentDescription.flags = 0;
		colorAttachmentDescription.format = surface.colorFormat.format;
		colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachmentDescription;
		depthAttachmentDescription.flags = 0;
		depthAttachmentDescription.format = physicalDevices[device.physicalDeviceIndex].depthFormat;
		depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentReference;
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentReference;
		depthAttachmentReference.attachment = 1;
		depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription renderSubpassDescription;
		renderSubpassDescription.flags = VK_RESERVED_FOR_FUTURE_USE;
		renderSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		renderSubpassDescription.inputAttachmentCount = 0;
		renderSubpassDescription.pInputAttachments = nullptr;
		renderSubpassDescription.colorAttachmentCount = 1;
		renderSubpassDescription.pColorAttachments = &colorAttachmentReference;
		renderSubpassDescription.pResolveAttachments = nullptr;
		renderSubpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
		renderSubpassDescription.preserveAttachmentCount = 0;
		renderSubpassDescription.pPreserveAttachments = nullptr;

		VkSubpassDependency renderSubpassDependency;
		renderSubpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		renderSubpassDependency.dstSubpass = 0;
		renderSubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		renderSubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		renderSubpassDependency.srcAccessMask = 0;
		renderSubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		renderSubpassDependency.dependencyFlags = 0;

		std::vector<VkAttachmentDescription> attachments = { colorAttachmentDescription, depthAttachmentDescription };

		VkSubpassDescription subpasses[] = { renderSubpassDescription };
		VkSubpassDependency subpassDependencies[] = { renderSubpassDependency };

		VkRenderPassCreateInfo renderPassCreateInfo;
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.pNext = nullptr;
		renderPassCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		renderPassCreateInfo.attachmentCount = (uint32_t)attachments.size();
		renderPassCreateInfo.pAttachments = attachments.data();
		renderPassCreateInfo.subpassCount = sizeof(subpasses) / sizeof(VkSubpassDescription);
		renderPassCreateInfo.pSubpasses = subpasses;
		renderPassCreateInfo.dependencyCount = sizeof(subpassDependencies) / sizeof(VkSubpassDependency);
		renderPassCreateInfo.pDependencies = subpassDependencies;

		VK_CHECK_RESULT(vkCreateRenderPass(device.handle, &renderPassCreateInfo, nullptr, &renderPass), renderPass, "vkCreateRenderPass");
	}

	/// DescriptorPool
	{
		VkDescriptorPoolSize cameraDescriptorPoolSize;
		cameraDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraDescriptorPoolSize.descriptorCount = 1;

		VkDescriptorPoolSize modelMatricesDescriptorPoolSize;
		modelMatricesDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		modelMatricesDescriptorPoolSize.descriptorCount = 1;

		VkDescriptorPoolSize pointLightDescriptorPoolSize;
		pointLightDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pointLightDescriptorPoolSize.descriptorCount = 1;

		VkDescriptorPoolSize textureDescriptorPoolSize;
		textureDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureDescriptorPoolSize.descriptorCount = 1;

		VkDescriptorPoolSize descriptorPoolSize[] = { cameraDescriptorPoolSize, modelMatricesDescriptorPoolSize, pointLightDescriptorPoolSize, textureDescriptorPoolSize };

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCreateInfo.pNext = nullptr;
		descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		descriptorPoolCreateInfo.maxSets = 1;
		descriptorPoolCreateInfo.poolSizeCount = sizeof(descriptorPoolSize) / sizeof(VkDescriptorPoolSize);
		descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize;

		VK_CHECK_RESULT(vkCreateDescriptorPool(device.handle, &descriptorPoolCreateInfo, nullptr, &descriptorPool), descriptorPool, "vkCreateDescriptorPool");
	}

	/// descriptorSet Layout
	{
		VkDescriptorSetLayoutBinding cameraDescriptorSetLayoutBinding;
		cameraDescriptorSetLayoutBinding.binding = 0;
		cameraDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		cameraDescriptorSetLayoutBinding.descriptorCount = 1;
		cameraDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		cameraDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding modelMatricesDescriptorSetLayoutBinding;
		modelMatricesDescriptorSetLayoutBinding.binding = 1;
		modelMatricesDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		modelMatricesDescriptorSetLayoutBinding.descriptorCount = 1;
		modelMatricesDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		modelMatricesDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding pointLightDescriptorSetLayoutBinding;
		pointLightDescriptorSetLayoutBinding.binding = 2;
		pointLightDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pointLightDescriptorSetLayoutBinding.descriptorCount = 1;
		pointLightDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pointLightDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding textureDescriptorSetLayoutBinding;
		textureDescriptorSetLayoutBinding.binding = 3;
		textureDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureDescriptorSetLayoutBinding.descriptorCount = 1;
		textureDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		textureDescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding descriptorSetLayoutBinding[] = { cameraDescriptorSetLayoutBinding, modelMatricesDescriptorSetLayoutBinding, textureDescriptorSetLayoutBinding, pointLightDescriptorSetLayoutBinding };

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pNext = nullptr;
		descriptorSetLayoutCreateInfo.flags = VK_NULL_HANDLE;
		descriptorSetLayoutCreateInfo.bindingCount = sizeof(descriptorSetLayoutBinding) / sizeof(VkDescriptorSetLayoutBinding);
		descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBinding;

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device.handle, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout), descriptorSetLayout, "vkCreateDescriptorSetLayout");
	}

	/// DescriptorSet
	{
		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAllocateInfo.pNext = nullptr;
		descriptorSetAllocateInfo.descriptorPool = descriptorPool;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;

		VK_CHECK_RESULT(vkAllocateDescriptorSets(device.handle, &descriptorSetAllocateInfo, &descriptorSet), descriptorSet, "vkAllocateDescriptorSets");
	}

	/// Sampler
	{
		VkSamplerCreateInfo samplerCreateInfo;
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = nullptr;
		samplerCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.anisotropyEnable = VK_TRUE;
		samplerCreateInfo.maxAnisotropy = 16;
		samplerCreateInfo.compareEnable = VK_FALSE;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

		VK_CHECK_RESULT(vkCreateSampler(device.handle, &samplerCreateInfo, nullptr, &sampler), sampler, "vkCreateSampler");
	}

	/// Swapchain
	uint32_t targetSwapchainImageCount = 3;
	bool useDepthBuffer = true;
	{
		// swapchain
		{
			swapchain.extent.width = width;
			swapchain.extent.height = height;

			VkSurfaceCapabilitiesKHR surfaceCapabilities = VkU::GetVkSurfaceCapabilitiesKHR(physicalDevices[device.physicalDeviceIndex].handle, surface.handle);

			if (targetSwapchainImageCount > surfaceCapabilities.maxImageCount)
				targetSwapchainImageCount = surfaceCapabilities.maxImageCount;
			else if (targetSwapchainImageCount < surfaceCapabilities.minImageCount)
				targetSwapchainImageCount = surfaceCapabilities.minImageCount;

			if (swapchain.extent.width > surfaceCapabilities.maxImageExtent.width)
				swapchain.extent.width = surfaceCapabilities.maxImageExtent.width;
			else if (swapchain.extent.width < surfaceCapabilities.minImageExtent.width)
				swapchain.extent.width = surfaceCapabilities.minImageExtent.width;
			if (swapchain.extent.height > surfaceCapabilities.maxImageExtent.height)
				swapchain.extent.height = surfaceCapabilities.maxImageExtent.height;
			else if (swapchain.extent.height < surfaceCapabilities.minImageExtent.height)
				swapchain.extent.height = surfaceCapabilities.minImageExtent.height;

			VkSwapchainCreateInfoKHR swapchainCreateInfoKHR;
			swapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCreateInfoKHR.pNext = nullptr;
			swapchainCreateInfoKHR.flags = VK_RESERVED_FOR_FUTURE_USE;
			swapchainCreateInfoKHR.surface = surface.handle;
			swapchainCreateInfoKHR.minImageCount = targetSwapchainImageCount;
			swapchainCreateInfoKHR.imageFormat = surface.colorFormat.format;
			swapchainCreateInfoKHR.imageColorSpace = surface.colorFormat.colorSpace;
			swapchainCreateInfoKHR.imageExtent = swapchain.extent;
			swapchainCreateInfoKHR.imageArrayLayers = 1;
			swapchainCreateInfoKHR.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			swapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCreateInfoKHR.queueFamilyIndexCount = 0;
			swapchainCreateInfoKHR.pQueueFamilyIndices = nullptr;
			swapchainCreateInfoKHR.preTransform = surfaceCapabilities.currentTransform;
			swapchainCreateInfoKHR.compositeAlpha = surface.compositeAlpha;
			swapchainCreateInfoKHR.presentMode = surface.presentMode;
			swapchainCreateInfoKHR.clipped = VK_TRUE;
			swapchainCreateInfoKHR.oldSwapchain = VK_NULL_HANDLE;

			VK_CHECK_RESULT(vkCreateSwapchainKHR(device.handle, &swapchainCreateInfoKHR, nullptr, &swapchain.handle), swapchain.handle, "vkCreateSwapchainKHR");
		}

		// image
		{
			uint32_t propertyCount = 0;
			VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &propertyCount, nullptr), "????????????????", "vkGetSwapchainImagesKHR");
			swapchain.images.resize(propertyCount);
			VK_CHECK_RESULT(vkGetSwapchainImagesKHR(device.handle, swapchain.handle, &propertyCount, swapchain.images.data()), "????????????????", "vkGetSwapchainImagesKHR");
		}

		// view
		{
			VkImageViewCreateInfo imageViewCreateInfo;
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.pNext = nullptr;
			imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
			imageViewCreateInfo.image = VK_NULL_HANDLE;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = surface.colorFormat.format;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;

			swapchain.views.resize(swapchain.images.size());
			for (size_t i = 0; i != swapchain.views.size(); ++i)
			{
				imageViewCreateInfo.image = swapchain.images[i];
				VK_CHECK_RESULT(vkCreateImageView(device.handle, &imageViewCreateInfo, nullptr, &swapchain.views[i]), swapchain.views[i], "vkCreateImageView");
			}
		}

		// Depth Image
		{
			depthImage.handle = VK_NULL_HANDLE;
			depthImage.memory = VK_NULL_HANDLE;
			depthImage.view = VK_NULL_HANDLE;

			if (useDepthBuffer)
			{
				// create
				{
					// image
					{
						VkImageCreateInfo imageCreateInfo;
						imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
						imageCreateInfo.pNext = nullptr;
						imageCreateInfo.flags = 0;
						imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
						imageCreateInfo.format = physicalDevices[device.physicalDeviceIndex].depthFormat;
						imageCreateInfo.extent = { swapchain.extent.width, swapchain.extent.height, 1 };
						imageCreateInfo.mipLevels = 1;
						imageCreateInfo.arrayLayers = 1;
						imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
						imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
						imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
						imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
						imageCreateInfo.queueFamilyIndexCount = 0;
						imageCreateInfo.pQueueFamilyIndices = nullptr;
						imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

						VK_CHECK_RESULT(vkCreateImage(device.handle, &imageCreateInfo, nullptr, &depthImage.handle), depthImage.handle, "vkCreateImage");
					}

					// memory
					{
						VkMemoryRequirements memoryRequirements;
						vkGetImageMemoryRequirements(device.handle, depthImage.handle, &memoryRequirements);

						VkMemoryAllocateInfo memoryAllocateInfo;
						memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
						memoryAllocateInfo.pNext = nullptr;
						memoryAllocateInfo.allocationSize = memoryRequirements.size;
						memoryAllocateInfo.memoryTypeIndex = VkU::FindMemoryTypeIndex(memoryRequirements, physicalDevices[device.physicalDeviceIndex], VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
						VK_CHECK_RESULT(vkAllocateMemory(device.handle, &memoryAllocateInfo, nullptr, &depthImage.memory), depthImage.memory, "vkAllocateMemory");

						VK_CHECK_RESULT(vkBindImageMemory(device.handle, depthImage.handle, depthImage.memory, 0), "????????????????", "vkBindImageMemory");
					}

					// view
					{
						VkImageViewCreateInfo imageViewCreateInfo;
						imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
						imageViewCreateInfo.pNext = nullptr;
						imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
						imageViewCreateInfo.image = depthImage.handle;
						imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
						imageViewCreateInfo.format = physicalDevices[device.physicalDeviceIndex].depthFormat;
						imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
						imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
						imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
						imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
						imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
						imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
						imageViewCreateInfo.subresourceRange.layerCount = 1;
						imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
						imageViewCreateInfo.subresourceRange.levelCount = 1;

						VK_CHECK_RESULT(vkCreateImageView(device.handle, &imageViewCreateInfo, nullptr, &depthImage.view), depthImage.view, "vkCreateImageView");
					}
				}

				// transfer layout
				{
					// begin command buffer
					{
						VkCommandBufferBeginInfo commandBufferBeginInfo;
						commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
						commandBufferBeginInfo.pNext = nullptr;
						commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
						commandBufferBeginInfo.pInheritanceInfo = nullptr;
						VkU::WaitResetFence(device.handle, 1, &setupFence, VK_TRUE, -1);
						VK_CHECK_RESULT(vkBeginCommandBuffer(setupCommandBuffer, &commandBufferBeginInfo), "????????????????", "vkBeginCommandBuffer");
					}

					// pipeline barrier
					{
						VkImageMemoryBarrier imageMemoryBarrier;
						imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
						imageMemoryBarrier.pNext = nullptr;
						imageMemoryBarrier.srcAccessMask = 0;
						imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
						imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
						imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
						imageMemoryBarrier.image = depthImage.handle;
						imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
						imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
						imageMemoryBarrier.subresourceRange.levelCount = 1;
						imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
						imageMemoryBarrier.subresourceRange.layerCount = 1;

						vkCmdPipelineBarrier(setupCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
					}

					// end command buffer
					{
						VK_CHECK_RESULT(vkEndCommandBuffer(setupCommandBuffer), "????????????????", "vkEndCommandBuffer");
					}

					// submit command buffer
					{
						VkSubmitInfo submitInfo;
						submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
						submitInfo.pNext = nullptr;
						submitInfo.waitSemaphoreCount = 0;
						submitInfo.pWaitSemaphores = nullptr;
						submitInfo.pWaitDstStageMask = nullptr;
						submitInfo.commandBufferCount = 1;
						submitInfo.pCommandBuffers = &setupCommandBuffer;
						submitInfo.signalSemaphoreCount = 0;
						submitInfo.pSignalSemaphores = nullptr;

						VK_CHECK_RESULT(vkQueueSubmit(device.queues[GRAPHICS_PRESENT_QUEUE_INDEX].handles[0], 1, &submitInfo, setupFence), "????????????????", "vkQueueSubmit");
					}
				}
			}
		}

		// framebuffer
		{{

				VkFramebufferCreateInfo framebufferCreateInfo;
				framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCreateInfo.pNext = nullptr;
				framebufferCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
				framebufferCreateInfo.renderPass = renderPass;
				framebufferCreateInfo.attachmentCount = 0;
				framebufferCreateInfo.pAttachments = nullptr;
				framebufferCreateInfo.width = swapchain.extent.width;
				framebufferCreateInfo.height = swapchain.extent.height;
				framebufferCreateInfo.layers = 1;

				swapchain.framebuffers.resize(swapchain.images.size());
				for (size_t i = 0; i != swapchain.framebuffers.size(); ++i)
				{
					std::vector<VkImageView> attachments;
					attachments.push_back(swapchain.views[i]);
					if (useDepthBuffer)
						attachments.push_back(depthImage.view);

					framebufferCreateInfo.attachmentCount = (uint32_t)attachments.size();
					framebufferCreateInfo.pAttachments = attachments.data();

					VK_CHECK_RESULT(vkCreateFramebuffer(device.handle, &framebufferCreateInfo, nullptr, &swapchain.framebuffers[i]), swapchain.framebuffers[i], "vkCreateFramebuffer");
				}
			}}
	}

	/// Semaphore
	{{
			VkSemaphoreCreateInfo semaphoreCreateInfo;
			semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			semaphoreCreateInfo.pNext = nullptr;
			semaphoreCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;

			VK_CHECK_RESULT(vkCreateSemaphore(device.handle, &semaphoreCreateInfo, nullptr, &semaphoreImageAvailable), semaphoreImageAvailable, "vkCreateSemaphore");
			VK_CHECK_RESULT(vkCreateSemaphore(device.handle, &semaphoreCreateInfo, nullptr, &semaphoreRenderDone), semaphoreRenderDone, "vkCreateSemaphore");
		}}

	/// render command buffer
	{
		renderCommandBuffers.resize(swapchain.framebuffers.size());

		VkCommandBufferAllocateInfo commandBufferAllocateInfo;
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.pNext = nullptr;
		commandBufferAllocateInfo.commandPool = commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = (uint32_t)renderCommandBuffers.size();

		VK_CHECK_RESULT(vkAllocateCommandBuffers(device.handle, &commandBufferAllocateInfo, renderCommandBuffers.data()), "????????????????", "vkAllocateCommandBuffers");
	}

	/// render fences
	{
		renderFences.resize(renderCommandBuffers.size());

		VkFenceCreateInfo fenceCreateInfo;
		fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCreateInfo.pNext = nullptr;
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i != renderFences.size(); ++i)
		{
			VK_CHECK_RESULT(vkCreateFence(device.handle, &fenceCreateInfo, nullptr, &renderFences[i]), renderFences[i], "vkCreateFence");
		}
	}
}
void Renderer::Load(std::vector<ShaderProperties> _shaderModulesProperties, std::vector<const char*> _modelNames, std::vector<const char*> _imageNames)
{
	maxGpuModelMatrixCount = 64;
	maxGpuPointLightCount = 4;

	/// uniforBuffers
	{
		// viewProjection
		{
			viewProjectionBuffer = VkU::CreateUniformBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], sizeof(viewProjection));
			viewProjectionStagingBuffer = VkU::CreateStagingBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], sizeof(viewProjection));
		}

		// model matrices
		{
			modelMatrices.resize(maxGpuModelMatrixCount);
			modelMatrices[1][3][0] = 3.0f;
			modelMatrices[1][3][1] = 3.0f;
			modelMatricesBuffer = VkU::CreateUniformBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], sizeof(glm::mat4) * maxGpuModelMatrixCount);
			modelMatricesStagingBuffer = VkU::CreateStagingBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], sizeof(glm::mat4) * maxGpuModelMatrixCount);
		}

		// point lights
		{
			pointLights.resize(maxGpuPointLightCount);
			pointLights[0].position = {10.0f, 10.0f, 0.0f};
			pointLights[0].padding = 3.0f;
			pointLights[0].color = {0.0f, 0.0f, 0.0f};
			pointLights[0].strenght = 0.0f;

			pointLights[1].position = {-10.0f, 10.0f, 0.0f};
			pointLights[1].padding = 11.0f;
			pointLights[1].color = {1.0f, 1.0f, 1.0f};
			pointLights[1].strenght = 0.0f;

			pointLights[2].position = {10.0f, -10.0f, 0.0f};
			pointLights[2].padding = 19.0f;
			pointLights[2].color = {0.0f, 0.0f, 0.0f};
			pointLights[2].strenght = 0.0f;

			pointLights[3].position = {-10.0f, -10.0f, 0.0f};
			pointLights[3].padding = 27.0f;
			pointLights[3].color = {0.0f, 0.0f, 0.0f};
			pointLights[3].strenght = 0.0f;

			pointLightsBuffer = VkU::CreateUniformBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], sizeof(VkU::PointLight) * pointLights.size());
			pointLightsStagingBuffer = VkU::CreateStagingBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], sizeof(VkU::PointLight) * pointLights.size());
		}
	}

	/// textures
	{{
		imageBuffers.resize(_imageNames.size());
		for (size_t i = 0; i != _imageNames.size(); ++i)
		{
			
			uint32_t width, height;
			uint8_t channelCount, bytesPerChannel;
			void* data = nullptr;
			VkFormat imageFormat;
			VkDeviceSize size;

			// create
			{
				// gather data
				VkU::LoadImageTGA(_imageNames[i], width, height, channelCount, bytesPerChannel, data);

				size = width * height * channelCount * bytesPerChannel;

				if (channelCount == 3)
					imageFormat = VK_FORMAT_B8G8R8_UNORM;
				else if (channelCount == 4)
					imageFormat = VK_FORMAT_B8G8R8A8_UNORM;

				// image
				VkU::CreateSampledImage(device.handle, physicalDevices[device.physicalDeviceIndex], imageBuffers[i], imageFormat, { width, height, 1 });
				// view
				VkU::CreateColorView(device.handle, imageBuffers[i], imageFormat);
			}

			VkU::Image stagingImage;
			// staging
			{
				VkU::CreateStagingImage(device.handle, physicalDevices[device.physicalDeviceIndex], stagingImage, imageFormat, { width, height, 1 });
				VkU::FillStagingColorImage(device.handle, stagingImage, width, height, size, data);
			}

			// transition
			{
				VkCommandBufferBeginInfo commandBufferBeginInfo;
				commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				commandBufferBeginInfo.pNext = nullptr;
				commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				commandBufferBeginInfo.pInheritanceInfo = nullptr;
				VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), 0, "vkWaitForFences");
				VK_CHECK_RESULT(vkBeginCommandBuffer(setupCommandBuffer, &commandBufferBeginInfo), 0, "vkBeginCommandBuffer");

				// transfer texture to destination
				VkImageMemoryBarrier imageMemoryBarrier;
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.pNext = nullptr;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imageMemoryBarrier.image = imageBuffers[i].handle;
				imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
				imageMemoryBarrier.subresourceRange.levelCount = 1;
				imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
				imageMemoryBarrier.subresourceRange.layerCount = 1;
				vkCmdPipelineBarrier(setupCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

				// transfer staging to source
				VkImageMemoryBarrier stagingImageMemoryBarrier;
				stagingImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				stagingImageMemoryBarrier.pNext = nullptr;
				stagingImageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				stagingImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				stagingImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
				stagingImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				stagingImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				stagingImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				stagingImageMemoryBarrier.image = stagingImage.handle;
				stagingImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				stagingImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
				stagingImageMemoryBarrier.subresourceRange.levelCount = 1;
				stagingImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
				stagingImageMemoryBarrier.subresourceRange.layerCount = 1;
				vkCmdPipelineBarrier(setupCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &stagingImageMemoryBarrier);

				// copy data 
				VkImageSubresourceLayers imageSubresourceLayers;
				imageSubresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageSubresourceLayers.mipLevel = 0;
				imageSubresourceLayers.baseArrayLayer = 0;
				imageSubresourceLayers.layerCount = 1;

				VkImageCopy imageCopy;
				imageCopy.srcSubresource = imageSubresourceLayers;
				imageCopy.srcOffset = { 0, 0, 0 };
				imageCopy.dstSubresource = imageSubresourceLayers;
				imageCopy.dstOffset = { 0, 0, 0 };
				imageCopy.extent.width = width;
				imageCopy.extent.height = height;
				imageCopy.extent.depth = 1;

				vkCmdCopyImage(setupCommandBuffer, stagingImage.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, imageBuffers[i].handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy);

				// transfer texture to shader readable layout
				VkImageMemoryBarrier finalMemoryBarrier;
				finalMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				finalMemoryBarrier.pNext = nullptr;
				finalMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				finalMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				finalMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				finalMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				finalMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				finalMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				finalMemoryBarrier.image = imageBuffers[i].handle;
				finalMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				finalMemoryBarrier.subresourceRange.baseMipLevel = 0;
				finalMemoryBarrier.subresourceRange.levelCount = 1;
				finalMemoryBarrier.subresourceRange.baseArrayLayer = 0;
				finalMemoryBarrier.subresourceRange.layerCount = 1;
				vkCmdPipelineBarrier(setupCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &finalMemoryBarrier);

				VK_CHECK_RESULT(vkEndCommandBuffer(setupCommandBuffer), 0, "vkEndCommandBuffer");

				VkSubmitInfo submitInfo;
				submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submitInfo.pNext = nullptr;
				submitInfo.waitSemaphoreCount = 0;
				submitInfo.pWaitSemaphores = nullptr;
				submitInfo.pWaitDstStageMask = nullptr;
				submitInfo.commandBufferCount = 1;
				submitInfo.pCommandBuffers = &setupCommandBuffer;
				submitInfo.signalSemaphoreCount = 0;
				submitInfo.pSignalSemaphores = nullptr;
				VK_CHECK_RESULT(vkQueueSubmit(device.queues[GRAPHICS_PRESENT_QUEUE_INDEX].handles[0], 1, &submitInfo, VK_NULL_HANDLE), 0, "vkQueueSubmit");
				VK_CHECK_RESULT(vkQueueWaitIdle(device.queues[GRAPHICS_PRESENT_QUEUE_INDEX].handles[0]), 0, "vkQueueWaitIdle");
			}

			vkDestroyImage(device.handle, stagingImage.handle, nullptr);
			vkFreeMemory(device.handle, stagingImage.memory, nullptr);
			delete[] data;
		}
	}}

	//std::vector<VkU::VertexPosUV> mesh = 
	//{
	//	{ { -1.0f, -1.0f, -1.0f },{ 0.0f, 0.0f } },
	//	{ { +1.0f, -1.0f, -1.0f },{ 1.0f, 0.0f } },
	//	{ { +1.0f, +1.0f, -1.0f },{ 1.0f, 1.0f } },
	//	{ { -1.0f, +1.0f, -1.0f },{ 0.0f, 1.0f } },
	//
	//	{ { -2.0f, -1.0f, +1.0f },{ 0.0f, 0.0f } },
	//	{ { +2.0f, -1.0f, +1.0f },{ 1.0f, 0.0f } },
	//	{ { +2.0f, +1.0f, +1.0f },{ 1.0f, 1.0f } },
	//	{ { -2.0f, +1.0f, +1.0f },{ 0.0f, 1.0f } },
	//};
	//std::vector<uint16_t> indices = 
	//{
	//	0, 1, 2, 2, 3, 0,
	//	4, 5, 6, 6, 7, 4,
	//};

	VkU::Meshes rmesh;
	VkU::LoadModel(_modelNames[0], rmesh, (aiPostProcessSteps)(aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices));

	/// vertexBuffer / indexBuffer
	{
		//VkDeviceSize vertexBufferSize = sizeof(VkU::VertexPosUV) * mesh.size();
		//VkDeviceSize indexBufferSize = sizeof(VkU::VertexPosUV) * mesh.size();
		//
		//vertexBuffer = VkU::CreateVertexBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], vertexBufferSize);
		//VkU::Buffer vertexStagingBuffer = VkU::CreateStagingBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], vertexBufferSize);
		//VkU::FillStagingBuffer(device.handle, vertexStagingBuffer, vertexBufferSize, mesh.data());
		//VkU::TransferStagingBuffer(device, setupCommandBuffer, setupFence, vertexStagingBuffer, vertexBuffer, vertexBufferSize);
		//VkU::WaitFence(device.handle, 1, &setupFence, VK_TRUE, -1);
		//VkU::DestroyBuffer(device.handle, vertexStagingBuffer);
		//
		//indexBuffer = VkU::CreateIndexBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], indexBufferSize);
		//VkU::Buffer indexStagingBuffer = VkU::CreateStagingBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], indexBufferSize);
		//VkU::FillStagingBuffer(device.handle, indexStagingBuffer, indexBufferSize, indices.data());
		//VkU::TransferStagingBuffer(device, setupCommandBuffer, setupFence, indexStagingBuffer, indexBuffer, indexBufferSize);
		//VkU::WaitFence(device.handle, 1, &setupFence, VK_TRUE, -1);
		//VkU::DestroyBuffer(device.handle, indexStagingBuffer);

		VkDeviceSize vertexBufferSize = rmesh.vertexSize;
		VkDeviceSize indexBufferSize = rmesh.indexSize;
		
		vertexBuffer = VkU::CreateVertexBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], vertexBufferSize);
		VkU::Buffer vertexStagingBuffer = VkU::CreateStagingBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], vertexBufferSize);
		VkU::FillStagingBuffer(device.handle, vertexStagingBuffer, vertexBufferSize, rmesh.vertexData);
		VkU::TransferStagingBuffer(device, setupCommandBuffer, setupFence, vertexStagingBuffer, vertexBuffer, vertexBufferSize);
		VkU::WaitFence(device.handle, 1, &setupFence, VK_TRUE, -1);
		VkU::DestroyBuffer(device.handle, vertexStagingBuffer);
		
		indexBuffer = VkU::CreateIndexBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], indexBufferSize);
		VkU::Buffer indexStagingBuffer = VkU::CreateStagingBuffer(device.handle, physicalDevices[device.physicalDeviceIndex], indexBufferSize);
		VkU::FillStagingBuffer(device.handle, indexStagingBuffer, indexBufferSize, rmesh.indexData);
		VkU::TransferStagingBuffer(device, setupCommandBuffer, setupFence, indexStagingBuffer, indexBuffer, indexBufferSize);
		VkU::WaitFence(device.handle, 1, &setupFence, VK_TRUE, -1);
		VkU::DestroyBuffer(device.handle, indexStagingBuffer);
	}

	delete[] rmesh.indexData;
	delete[] rmesh.vertexData;

	/// shaders modules
	{
		shaderModules.resize(_shaderModulesProperties.size());
		for (size_t i = 0; i != _shaderModulesProperties.size(); ++i)
		{
			// create
			{
				size_t fileSize = 0;
				char* buffer = nullptr;

				VkU::LoadShader(_shaderModulesProperties[i].filename, fileSize, &buffer);

				VkShaderModuleCreateInfo shaderModuleCreateInfo;
				shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleCreateInfo.pNext = nullptr;
				shaderModuleCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
				shaderModuleCreateInfo.codeSize = fileSize;
				shaderModuleCreateInfo.pCode = (uint32_t*)buffer;
				VK_CHECK_RESULT(vkCreateShaderModule(device.handle, &shaderModuleCreateInfo, nullptr, &shaderModules[i].handle), shaderModules[i].handle, "vkCreateShaderModule");

				delete[] buffer;

				shaderModules[i].stage = _shaderModulesProperties[i].stage;
				shaderModules[i].entryPointName = _shaderModulesProperties[i].entryPointName;
			}
		}
	}
}
void Renderer::Setup()
{
	/// pipeline layout
	{
		VkPushConstantRange vertexShaderPushConstantRange;
		vertexShaderPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		vertexShaderPushConstantRange.offset = 0;
		vertexShaderPushConstantRange.size = sizeof(float) * 4;

		VkPushConstantRange fragmentShaderPushConstantRange;
		fragmentShaderPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentShaderPushConstantRange.offset = 16;
		fragmentShaderPushConstantRange.size = sizeof(float) * 5;

		VkPushConstantRange pushConstantRanges[] = { vertexShaderPushConstantRange, fragmentShaderPushConstantRange };

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pNext = nullptr;
		pipelineLayoutCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutCreateInfo.pushConstantRangeCount = sizeof(pushConstantRanges) / sizeof(VkPushConstantRange);;
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges;

		VK_CHECK_RESULT(vkCreatePipelineLayout(device.handle, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout), pipelineLayout, "vkCreatePipelineLayout");
	}

	/// pipeline data
	{
		// shader stage
		{
			pipelineShaderStagesCreateInfos.resize(1);

			pipelineShaderStagesCreateInfos[0].resize(2);

			pipelineShaderStagesCreateInfos[0][0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipelineShaderStagesCreateInfos[0][0].pNext = nullptr;
			pipelineShaderStagesCreateInfos[0][0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineShaderStagesCreateInfos[0][0].stage = shaderModules[0].stage;
			pipelineShaderStagesCreateInfos[0][0].module = shaderModules[0].handle;
			pipelineShaderStagesCreateInfos[0][0].pName = shaderModules[0].entryPointName;
			pipelineShaderStagesCreateInfos[0][0].pSpecializationInfo = nullptr;

			pipelineShaderStagesCreateInfos[0][1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			pipelineShaderStagesCreateInfos[0][1].pNext = nullptr;
			pipelineShaderStagesCreateInfos[0][1].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineShaderStagesCreateInfos[0][1].stage = shaderModules[1].stage;
			pipelineShaderStagesCreateInfos[0][1].module = shaderModules[1].handle;
			pipelineShaderStagesCreateInfos[0][1].pName = shaderModules[1].entryPointName;
			pipelineShaderStagesCreateInfos[0][1].pSpecializationInfo = nullptr;
		}

		// vertex binding description
		{
			vertexInputBindingDescriptions.resize(1);

			vertexInputBindingDescriptions[0].binding = 0;
			vertexInputBindingDescriptions[0].stride = sizeof(VkU::VertexPosUvNormTanBitan);
			vertexInputBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}

		// vertex attribute description
		{
			vertexInputAttributeDescriptions.resize(1);

			vertexInputAttributeDescriptions[0].resize(5);

			vertexInputAttributeDescriptions[0][0].location = 0;
			vertexInputAttributeDescriptions[0][0].binding = 0;
			vertexInputAttributeDescriptions[0][0].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributeDescriptions[0][0].offset = offsetof(VkU::VertexPosUvNormTanBitan, position);

			vertexInputAttributeDescriptions[0][1].location = 1;
			vertexInputAttributeDescriptions[0][1].binding = 0;
			vertexInputAttributeDescriptions[0][1].format = VK_FORMAT_R32G32_SFLOAT;
			vertexInputAttributeDescriptions[0][1].offset = offsetof(VkU::VertexPosUvNormTanBitan, uv);

			vertexInputAttributeDescriptions[0][2].location = 2;
			vertexInputAttributeDescriptions[0][2].binding = 0;
			vertexInputAttributeDescriptions[0][2].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributeDescriptions[0][2].offset = offsetof(VkU::VertexPosUvNormTanBitan, normal);

			vertexInputAttributeDescriptions[0][3].location = 3;
			vertexInputAttributeDescriptions[0][3].binding = 0;
			vertexInputAttributeDescriptions[0][3].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributeDescriptions[0][3].offset = offsetof(VkU::VertexPosUvNormTanBitan, tangent);

			vertexInputAttributeDescriptions[0][4].location = 4;
			vertexInputAttributeDescriptions[0][4].binding = 0;
			vertexInputAttributeDescriptions[0][4].format = VK_FORMAT_R32G32B32_SFLOAT;
			vertexInputAttributeDescriptions[0][4].offset = offsetof(VkU::VertexPosUvNormTanBitan, bitangent);
		}

		// vertex Input stage
		{
			pipelineVertexInputStateCreateInfos.resize(1);

			pipelineVertexInputStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			pipelineVertexInputStateCreateInfos[0].pNext = nullptr;
			pipelineVertexInputStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineVertexInputStateCreateInfos[0].vertexBindingDescriptionCount = 1;
			pipelineVertexInputStateCreateInfos[0].pVertexBindingDescriptions = &vertexInputBindingDescriptions[0];
			pipelineVertexInputStateCreateInfos[0].vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributeDescriptions[0].size();
			pipelineVertexInputStateCreateInfos[0].pVertexAttributeDescriptions = vertexInputAttributeDescriptions[0].data();
		}

		// input assembly stage
		{
			pipelineInputAssemblyStateCreateInfos.resize(1);

			pipelineInputAssemblyStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			pipelineInputAssemblyStateCreateInfos[0].pNext = nullptr;
			pipelineInputAssemblyStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineInputAssemblyStateCreateInfos[0].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			pipelineInputAssemblyStateCreateInfos[0].primitiveRestartEnable = VK_FALSE;
		}

		// tessalation stage
		pipelineTessellationStateCreateInfos.resize(0);

		// viewports
		{
			viewports.resize(1);

			viewports[0].x = 0.0f;
			viewports[0].y = 0.0f;
			viewports[0].width = (float)swapchain.extent.width;
			viewports[0].height = (float)swapchain.extent.height;
			viewports[0].minDepth = 0.0f;
			viewports[0].maxDepth = 1.0f;
		}

		// scissors
		{
			scissors.resize(1);

			scissors[0].offset = { 0, 0 };
			scissors[0].extent = swapchain.extent;
		}

		// viewport State
		{
			pipelineViewportStateCreateInfos.resize(1);

			pipelineViewportStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			pipelineViewportStateCreateInfos[0].pNext = nullptr;
			pipelineViewportStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineViewportStateCreateInfos[0].viewportCount = 1;
			pipelineViewportStateCreateInfos[0].pViewports = &viewports[0];
			pipelineViewportStateCreateInfos[0].scissorCount = 1;
			pipelineViewportStateCreateInfos[0].pScissors = &scissors[0];
		}

		// rasterization state
		{
			pipelineRasterizationStateCreateInfos.resize(2);

			pipelineRasterizationStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			pipelineRasterizationStateCreateInfos[0].pNext = nullptr;
			pipelineRasterizationStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineRasterizationStateCreateInfos[0].depthClampEnable = VK_FALSE;
			pipelineRasterizationStateCreateInfos[0].rasterizerDiscardEnable = VK_FALSE;
			pipelineRasterizationStateCreateInfos[0].polygonMode = VK_POLYGON_MODE_FILL;
			pipelineRasterizationStateCreateInfos[0].cullMode = VK_CULL_MODE_BACK_BIT;
			pipelineRasterizationStateCreateInfos[0].frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			pipelineRasterizationStateCreateInfos[0].depthBiasEnable = VK_FALSE;
			pipelineRasterizationStateCreateInfos[0].depthBiasConstantFactor = 0.0f;
			pipelineRasterizationStateCreateInfos[0].depthBiasClamp = 0.0f;
			pipelineRasterizationStateCreateInfos[0].depthBiasSlopeFactor = 0.0f;
			pipelineRasterizationStateCreateInfos[0].lineWidth = 1.0f;

			pipelineRasterizationStateCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			pipelineRasterizationStateCreateInfos[1].pNext = nullptr;
			pipelineRasterizationStateCreateInfos[1].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineRasterizationStateCreateInfos[1].depthClampEnable = VK_FALSE;
			pipelineRasterizationStateCreateInfos[1].rasterizerDiscardEnable = VK_FALSE;
			pipelineRasterizationStateCreateInfos[1].polygonMode = VK_POLYGON_MODE_LINE;
			pipelineRasterizationStateCreateInfos[1].cullMode = VK_CULL_MODE_NONE;
			pipelineRasterizationStateCreateInfos[1].frontFace = VK_FRONT_FACE_CLOCKWISE;
			pipelineRasterizationStateCreateInfos[1].depthBiasEnable = VK_FALSE;
			pipelineRasterizationStateCreateInfos[1].depthBiasConstantFactor = 0.0f;
			pipelineRasterizationStateCreateInfos[1].depthBiasClamp = 0.0f;
			pipelineRasterizationStateCreateInfos[1].depthBiasSlopeFactor = 0.0f;
			pipelineRasterizationStateCreateInfos[1].lineWidth = 1.0f;
		}

		// multisample state
		{
			pipelineMultisampleStateCreateInfos.resize(1);

			pipelineMultisampleStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			pipelineMultisampleStateCreateInfos[0].pNext = nullptr;
			pipelineMultisampleStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineMultisampleStateCreateInfos[0].rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			pipelineMultisampleStateCreateInfos[0].sampleShadingEnable = VK_FALSE;
			pipelineMultisampleStateCreateInfos[0].minSampleShading = 0.0f;
			pipelineMultisampleStateCreateInfos[0].pSampleMask = nullptr;
			pipelineMultisampleStateCreateInfos[0].alphaToCoverageEnable = VK_FALSE;
			pipelineMultisampleStateCreateInfos[0].alphaToOneEnable = VK_FALSE;
		}

		// depth stencil state
		{
			pipelineDepthStencilStateCreateInfos.resize(1);

			pipelineDepthStencilStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			pipelineDepthStencilStateCreateInfos[0].pNext = nullptr;
			pipelineDepthStencilStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineDepthStencilStateCreateInfos[0].depthTestEnable = VK_TRUE;
			pipelineDepthStencilStateCreateInfos[0].depthWriteEnable = VK_TRUE;
			pipelineDepthStencilStateCreateInfos[0].depthCompareOp = VK_COMPARE_OP_LESS;
			pipelineDepthStencilStateCreateInfos[0].depthBoundsTestEnable = VK_FALSE;
			pipelineDepthStencilStateCreateInfos[0].stencilTestEnable = VK_FALSE;
			pipelineDepthStencilStateCreateInfos[0].front = {};
			pipelineDepthStencilStateCreateInfos[0].back = {};
			pipelineDepthStencilStateCreateInfos[0].minDepthBounds = 0.0f;
			pipelineDepthStencilStateCreateInfos[0].maxDepthBounds = 1.0f;
		}

		// Color Blend attachment State
		{
			pipelineColorBlendAttachmentState.resize(1);

			pipelineColorBlendAttachmentState[0].blendEnable = VK_FALSE;
			pipelineColorBlendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			pipelineColorBlendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
			pipelineColorBlendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
			pipelineColorBlendAttachmentState[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			pipelineColorBlendAttachmentState[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			pipelineColorBlendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;
			pipelineColorBlendAttachmentState[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		}

		// color blend state
		{
			pipelineColorBlendStateCreateInfos.resize(1);

			pipelineColorBlendStateCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			pipelineColorBlendStateCreateInfos[0].pNext = nullptr;
			pipelineColorBlendStateCreateInfos[0].flags = VK_RESERVED_FOR_FUTURE_USE;
			pipelineColorBlendStateCreateInfos[0].logicOpEnable = VK_FALSE;
			pipelineColorBlendStateCreateInfos[0].logicOp = VK_LOGIC_OP_COPY;
			pipelineColorBlendStateCreateInfos[0].attachmentCount = 1;
			pipelineColorBlendStateCreateInfos[0].pAttachments = &pipelineColorBlendAttachmentState[0];
			pipelineColorBlendStateCreateInfos[0].blendConstants[0] = 0.0f;
			pipelineColorBlendStateCreateInfos[0].blendConstants[1] = 0.0f;
			pipelineColorBlendStateCreateInfos[0].blendConstants[2] = 0.0f;
			pipelineColorBlendStateCreateInfos[0].blendConstants[3] = 0.0f;
		}

		// dynamic state
		pipelineDynamicStateCreateInfo.resize(0);

		// pipeline
		{
			graphicsPipelineCreateInfos.resize(2);

			graphicsPipelineCreateInfos[0].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicsPipelineCreateInfos[0].pNext = nullptr;
			graphicsPipelineCreateInfos[0].flags = 0;
			graphicsPipelineCreateInfos[0].stageCount = (uint32_t)pipelineShaderStagesCreateInfos[0].size();
			graphicsPipelineCreateInfos[0].pStages = pipelineShaderStagesCreateInfos[0].data();
			graphicsPipelineCreateInfos[0].pVertexInputState = &pipelineVertexInputStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pInputAssemblyState = &pipelineInputAssemblyStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pTessellationState = nullptr;
			graphicsPipelineCreateInfos[0].pViewportState = &pipelineViewportStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pRasterizationState = &pipelineRasterizationStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pMultisampleState = &pipelineMultisampleStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pDepthStencilState = &pipelineDepthStencilStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pColorBlendState = &pipelineColorBlendStateCreateInfos[0];
			graphicsPipelineCreateInfos[0].pDynamicState = nullptr;
			graphicsPipelineCreateInfos[0].layout = pipelineLayout;
			graphicsPipelineCreateInfos[0].renderPass = renderPass;
			graphicsPipelineCreateInfos[0].subpass = 0;
			graphicsPipelineCreateInfos[0].basePipelineHandle = VK_NULL_HANDLE;
			graphicsPipelineCreateInfos[0].basePipelineIndex = 0;

			graphicsPipelineCreateInfos[1].sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicsPipelineCreateInfos[1].pNext = nullptr;
			graphicsPipelineCreateInfos[1].flags = 0;
			graphicsPipelineCreateInfos[1].stageCount = (uint32_t)pipelineShaderStagesCreateInfos[0].size();
			graphicsPipelineCreateInfos[1].pStages = pipelineShaderStagesCreateInfos[0].data();
			graphicsPipelineCreateInfos[1].pVertexInputState = &pipelineVertexInputStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pInputAssemblyState = &pipelineInputAssemblyStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pTessellationState = nullptr;
			graphicsPipelineCreateInfos[1].pViewportState = &pipelineViewportStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pRasterizationState = &pipelineRasterizationStateCreateInfos[1];
			graphicsPipelineCreateInfos[1].pMultisampleState = &pipelineMultisampleStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pDepthStencilState = &pipelineDepthStencilStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pColorBlendState = &pipelineColorBlendStateCreateInfos[0];
			graphicsPipelineCreateInfos[1].pDynamicState = nullptr;
			graphicsPipelineCreateInfos[1].layout = pipelineLayout;
			graphicsPipelineCreateInfos[1].renderPass = renderPass;
			graphicsPipelineCreateInfos[1].subpass = 0;
			graphicsPipelineCreateInfos[1].basePipelineHandle = VK_NULL_HANDLE;
			graphicsPipelineCreateInfos[1].basePipelineIndex = 0;
		}
	}

	/// pipeline
	{{
		pipelines.resize(2);
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device.handle, VK_NULL_HANDLE, (uint32_t)graphicsPipelineCreateInfos.size(), graphicsPipelineCreateInfos.data(), nullptr, pipelines.data()), "????????????????", " - vkCreateGraphicsPipelines");
	}}

	/// update descriptor set
	{
		VkDescriptorBufferInfo cameraDescriptorBufferInfo;
		cameraDescriptorBufferInfo.buffer = viewProjectionBuffer.handle;
		cameraDescriptorBufferInfo.offset = 0;
		cameraDescriptorBufferInfo.range = sizeof(viewProjection);

		VkWriteDescriptorSet viewProjectionWriteDescriptorSet;
		viewProjectionWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		viewProjectionWriteDescriptorSet.pNext = nullptr;
		viewProjectionWriteDescriptorSet.dstSet = descriptorSet;
		viewProjectionWriteDescriptorSet.dstBinding = VIEW_PROJECTION_UNIFORM_BINDING;
		viewProjectionWriteDescriptorSet.dstArrayElement = 0;
		viewProjectionWriteDescriptorSet.descriptorCount = 1;
		viewProjectionWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		viewProjectionWriteDescriptorSet.pImageInfo = nullptr;
		viewProjectionWriteDescriptorSet.pBufferInfo = &cameraDescriptorBufferInfo;
		viewProjectionWriteDescriptorSet.pTexelBufferView = nullptr;

		VkDescriptorBufferInfo modelMatricesDescriptorBufferInfo;
		modelMatricesDescriptorBufferInfo.buffer = modelMatricesBuffer.handle;
		modelMatricesDescriptorBufferInfo.offset = 0;
		modelMatricesDescriptorBufferInfo.range = sizeof(glm::mat4) * modelMatrices.size();

		VkWriteDescriptorSet modelMatricesWriteDescriptorSet;
		modelMatricesWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		modelMatricesWriteDescriptorSet.pNext = nullptr;
		modelMatricesWriteDescriptorSet.dstSet = descriptorSet;
		modelMatricesWriteDescriptorSet.dstBinding = MODEL_MATRICES_UNIFORM_BINDING;
		modelMatricesWriteDescriptorSet.dstArrayElement = 0;
		modelMatricesWriteDescriptorSet.descriptorCount = 1;
		modelMatricesWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		modelMatricesWriteDescriptorSet.pImageInfo = nullptr;
		modelMatricesWriteDescriptorSet.pBufferInfo = &modelMatricesDescriptorBufferInfo;
		modelMatricesWriteDescriptorSet.pTexelBufferView = nullptr;

		VkDescriptorBufferInfo pointLightsDescriptorBufferInfo;
		pointLightsDescriptorBufferInfo.buffer = pointLightsBuffer.handle;
		pointLightsDescriptorBufferInfo.offset = 0;
		pointLightsDescriptorBufferInfo.range = sizeof(VkU::PointLight) * pointLights.size();

		VkWriteDescriptorSet pointLightsWriteDescriptorSet;
		pointLightsWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		pointLightsWriteDescriptorSet.pNext = nullptr;
		pointLightsWriteDescriptorSet.dstSet = descriptorSet;
		pointLightsWriteDescriptorSet.dstBinding = POINT_LIGHT_UNIFORM_BINDING;
		pointLightsWriteDescriptorSet.dstArrayElement = 0;
		pointLightsWriteDescriptorSet.descriptorCount = 1;
		pointLightsWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pointLightsWriteDescriptorSet.pImageInfo = nullptr;
		pointLightsWriteDescriptorSet.pBufferInfo = &pointLightsDescriptorBufferInfo;
		pointLightsWriteDescriptorSet.pTexelBufferView = nullptr;

		VkDescriptorImageInfo textureDescriptorImageInfo;
		textureDescriptorImageInfo.sampler = sampler;
		textureDescriptorImageInfo.imageView = imageBuffers[1].view;
		textureDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet textureWriteDescriptorSet;
		textureWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		textureWriteDescriptorSet.pNext = nullptr;
		textureWriteDescriptorSet.dstSet = descriptorSet;
		textureWriteDescriptorSet.dstBinding = TEXTURE_UNIFORM_BINDING;
		textureWriteDescriptorSet.dstArrayElement = 0;
		textureWriteDescriptorSet.descriptorCount = 1;
		textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureWriteDescriptorSet.pImageInfo = &textureDescriptorImageInfo;
		textureWriteDescriptorSet.pBufferInfo = nullptr;
		textureWriteDescriptorSet.pTexelBufferView = nullptr;

		VkWriteDescriptorSet writeDescriptorSet[] = { viewProjectionWriteDescriptorSet, modelMatricesWriteDescriptorSet, pointLightsWriteDescriptorSet, textureWriteDescriptorSet };

		vkUpdateDescriptorSets(device.handle, sizeof(writeDescriptorSet) / sizeof(VkWriteDescriptorSet), writeDescriptorSet, 0, nullptr);
	}
}
void Renderer::Render()
{
	// Prepare To Draw
	{
		// Get Swapchain Image Index
		VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &setupFence, VK_TRUE, -1), "????????????????", "vkWaitForFences");
		VK_CHECK_RESULT(vkResetFences(device.handle, 1, &setupFence), "????????????????", "vkResetFences");
		VK_CHECK_RESULT(vkAcquireNextImageKHR(device.handle, swapchain.handle, -1, semaphoreImageAvailable, setupFence, &swapchainImageIndex), swapchainImageIndex, "vkAcquireNextImageKHR");

		// Prepare buffer
		VkCommandBufferBeginInfo commandBufferBeginInfo;
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.pNext = nullptr;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		commandBufferBeginInfo.pInheritanceInfo = nullptr;
		VK_CHECK_RESULT(vkWaitForFences(device.handle, 1, &renderFences[swapchainImageIndex], VK_TRUE, -1), "????????????????", "vkWaitForFences");
		VK_CHECK_RESULT(vkResetFences(device.handle, 1, &renderFences[swapchainImageIndex]), "????????????????", "vkResetFences");
		VK_CHECK_RESULT(vkBeginCommandBuffer(renderCommandBuffers[swapchainImageIndex], &commandBufferBeginInfo), "????????????????", "vkBeginCommandBuffer");

		VkClearValue clearColor[2];
		clearColor[0].color = { 0.15f, 0.2f, 0.25f, 1.0f };
		clearColor[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.framebuffer = swapchain.framebuffers[swapchainImageIndex];
		renderPassBeginInfo.renderArea.offset = { 0, 0 };
		renderPassBeginInfo.renderArea.extent = { swapchain.extent.width, swapchain.extent.height };
		renderPassBeginInfo.clearValueCount = sizeof(clearColor) / sizeof(VkClearValue);
		renderPassBeginInfo.pClearValues = clearColor;
		vkCmdBeginRenderPass(renderCommandBuffers[swapchainImageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	// Draw
	{
		struct VertexShaderPushConstantData
		{
			uint32_t modelIndex;
			float red;
			float green;
			float blue;

		} vertexShaderPushConstantData;

		struct FragmentShaderPushConstantData
		{
			uint32_t lightCount;
			uint32_t lightIndex0;
			uint32_t lightIndex1;
			uint32_t lightIndex2;
			uint32_t lightIndex3;

		} fragmentShaderPushConstantData;

		VkDeviceSize offset = 0;

		float time = (float)Engine::timer.GetTime() + 3.0f;

		vkCmdBindPipeline(renderCommandBuffers[swapchainImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[0]);

		vkCmdBindVertexBuffers(renderCommandBuffers[swapchainImageIndex], 0, 1, &vertexBuffer.handle, &offset);
		vkCmdBindIndexBuffer(renderCommandBuffers[swapchainImageIndex], indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(renderCommandBuffers[swapchainImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
		vertexShaderPushConstantData =
		{
			0,
			sin(time) * 10,
			10.0f,
			cos(time) * 10,
		};

		fragmentShaderPushConstantData =
		{
			4,
			0,
			1,
			2,
			3,
		};
		vkCmdPushConstants(renderCommandBuffers[swapchainImageIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertexShaderPushConstantData), &vertexShaderPushConstantData);
		vkCmdPushConstants(renderCommandBuffers[swapchainImageIndex], pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 16, sizeof(fragmentShaderPushConstantData), &fragmentShaderPushConstantData);
		vkCmdDrawIndexed(renderCommandBuffers[swapchainImageIndex], 320*3, 1, 0, 0, 0);

		vertexShaderPushConstantData =
		{
			1,
			sin(time) * 10,
			10.0f,
			cos(time) * 10,
		};
		vkCmdPushConstants(renderCommandBuffers[swapchainImageIndex], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vertexShaderPushConstantData), &vertexShaderPushConstantData);
		vkCmdDrawIndexed(renderCommandBuffers[swapchainImageIndex], 320 * 3, 1, 0, 0, 0);
	}

	// draw conclusion
	{{
			vkCmdEndRenderPass(renderCommandBuffers[swapchainImageIndex]);
			VK_CHECK_RESULT(vkEndCommandBuffer(renderCommandBuffers[swapchainImageIndex]), "????????????????", "vkEndCommandBuffer");
	}}

	// Update uniforms
	{
		// data
		//viewProjection[0] = glm::lookAt(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		viewProjection[1] = glm::perspective(glm::radians(45.0f), swapchain.extent.width / (float)swapchain.extent.height, 0.1f, 1000.0f);
		viewProjection[1][1][1] *= -1;

		// staging
		VkU::FillStagingBuffer(device.handle, viewProjectionStagingBuffer, sizeof(viewProjection), viewProjection);
		VkU::TransferStagingBuffer(device, setupCommandBuffer, setupFence, viewProjectionStagingBuffer, viewProjectionBuffer, sizeof(viewProjection));

		modelMatrices[0] = glm::rotate(modelMatrices[0], (float)Engine::deltaTime/5, glm::vec3(0.0f, -1.0f, 0.0f));
		VkU::FillStagingBuffer(device.handle, modelMatricesStagingBuffer, sizeof(glm::mat4) * maxGpuModelMatrixCount, modelMatrices.data());
		VkU::TransferStagingBuffer(device, setupCommandBuffer, setupFence, modelMatricesStagingBuffer, modelMatricesBuffer, sizeof(glm::mat4) * maxGpuModelMatrixCount);

		VkU::FillStagingBuffer(device.handle, pointLightsStagingBuffer, sizeof(VkU::PointLight) * pointLights.size(), pointLights.data());
		VkU::TransferStagingBuffer(device, setupCommandBuffer, setupFence, pointLightsStagingBuffer, pointLightsBuffer, sizeof(VkU::PointLight) * pointLights.size());
	}

	// Handle Window
	{
		//gRenderer = this;
		++frameCount;
		sumFPS += (int)(1 / (Engine::timer.GetTime() - lastTime));

		SetWindowText(window.hWnd, (std::to_string((int)(1 / (Engine::timer.GetTime() - lastTime))) + std::string(" - FPS    ") + std::to_string(sumFPS / frameCount) + std::string(" - AVG")).c_str());

		lastTime = Engine::timer.GetTime();
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// Render
	{
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &semaphoreImageAvailable;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &renderCommandBuffers[swapchainImageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &semaphoreRenderDone;

		VK_CHECK_RESULT(vkQueueSubmit(device.queues[GRAPHICS_PRESENT_QUEUE_INDEX].handles[0], 1, &submitInfo, renderFences[swapchainImageIndex]), "????????????????", "vkQueueSubmit");

		VkPresentInfoKHR presentInfoKHR;
		presentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfoKHR.pNext = nullptr;
		presentInfoKHR.waitSemaphoreCount = 1;
		presentInfoKHR.pWaitSemaphores = &semaphoreRenderDone;
		presentInfoKHR.swapchainCount = 1;
		presentInfoKHR.pSwapchains = &swapchain.handle;
		presentInfoKHR.pImageIndices = &swapchainImageIndex;
		presentInfoKHR.pResults = nullptr;

		VK_CHECK_RESULT(vkQueuePresentKHR(device.queues[GRAPHICS_PRESENT_QUEUE_INDEX].handles[0], &presentInfoKHR), "????????????????", "vkQueuePresentKHR");
	}
}
void Renderer::ShutDown()
{
	vkDeviceWaitIdle(device.handle);

	// vertexBuffers / indexBuffers
	VkU::DestroyBuffer(device.handle, indexBuffer);
	VkU::DestroyBuffer(device.handle, vertexBuffer);

	// textures
	for (size_t i = 0; i != imageBuffers.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyImageView(device.handle, imageBuffers[i].view, nullptr), imageBuffers[i].view, "vkDestroyImageView");
		VK_CHECK_CLEANUP(vkDestroyImage(device.handle, imageBuffers[i].handle, nullptr), imageBuffers[i].handle, "vkDestroyBuffer");
		VK_CHECK_CLEANUP(vkFreeMemory(device.handle, imageBuffers[i].memory, nullptr), imageBuffers[i].memory, "vkFreeMemory");
	}
	imageBuffers.clear();

	// shader modules
	for (size_t i = 0; i != shaderModules.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyShaderModule(device.handle, shaderModules[i].handle, nullptr), shaderModules[i].handle, "vkDestroyShaderModule");
	}
	shaderModules.clear();

	// pointLights
	VkU::DestroyBuffer(device.handle, pointLightsBuffer);
	VkU::DestroyBuffer(device.handle, pointLightsStagingBuffer);

	// model matrices
	VkU::DestroyBuffer(device.handle, modelMatricesBuffer);
	VkU::DestroyBuffer(device.handle, modelMatricesStagingBuffer);

	// camera
	VkU::DestroyBuffer(device.handle, viewProjectionBuffer);
	VkU::DestroyBuffer(device.handle, viewProjectionStagingBuffer);

	//pipelines
	for (size_t i = 0; i != pipelines.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyPipeline(device.handle, pipelines[i], nullptr), pipelines[i], "vkDestroyPipeline");
	}
	pipelines.clear();

	// pipeline layout
	VK_CHECK_CLEANUP(vkDestroyPipelineLayout(device.handle, pipelineLayout, nullptr), pipelineLayout, "vkDestroyPipelineLayout");

	// descriptorSet Layout
	VK_CHECK_CLEANUP(vkDestroyDescriptorSetLayout(device.handle, descriptorSetLayout, nullptr), descriptorSetLayout, "vkDestroyDescriptorSetLayout");

	// render fences
	for (size_t i = 0; i != renderFences.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyFence(device.handle, renderFences[i], nullptr), renderFences[i], "vkDestroyFence");
	}
	renderFences.clear();

	// semaphores
	VK_CHECK_CLEANUP(vkDestroySemaphore(device.handle, semaphoreImageAvailable, nullptr), semaphoreImageAvailable, "vkDestroySemaphore");
	VK_CHECK_CLEANUP(vkDestroySemaphore(device.handle, semaphoreRenderDone, nullptr), semaphoreRenderDone, "vkDestroySemaphore");

	// swapchain
	for (size_t i = 0; i != swapchain.framebuffers.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyFramebuffer(device.handle, swapchain.framebuffers[i], nullptr), swapchain.framebuffers[i], "vkDestroyFramebuffer");
	}
	swapchain.framebuffers.clear();
	for (size_t i = 0; i != swapchain.views.size(); ++i)
	{
		VK_CHECK_CLEANUP(vkDestroyImageView(device.handle, swapchain.views[i], nullptr), swapchain.views[i], "vkDestroyImageView");
	}
	VK_CHECK_CLEANUP(vkDestroySwapchainKHR(device.handle, swapchain.handle, nullptr), swapchain.handle, "vkDestroySwapchainKHR");
	swapchain.views.clear();

	// depth Image
	if (depthImage.view != nullptr)
		VK_CHECK_CLEANUP(vkDestroyImageView(device.handle, depthImage.view, nullptr), depthImage.view, "vkDestroyImageView");
	if (depthImage.handle != nullptr)
		VK_CHECK_CLEANUP(vkDestroyImage(device.handle, depthImage.handle, nullptr), depthImage.handle, "vkDestroyBuffer");
	if (depthImage.memory != nullptr)
		VK_CHECK_CLEANUP(vkFreeMemory(device.handle, depthImage.memory, nullptr), depthImage.memory, "vkFreeMemory");

	// sampler
	VK_CHECK_CLEANUP(vkDestroySampler(device.handle, sampler, nullptr), sampler, "vkDestroySampler");

	// descriptorPool
	VK_CHECK_CLEANUP(vkDestroyDescriptorPool(device.handle, descriptorPool, nullptr), descriptorPool, "vkDestroyDescriptorSetLayout");

	// setup fence
	VK_CHECK_CLEANUP(vkDestroyFence(device.handle, setupFence, nullptr), setupFence, "vkDestroyFence");

	// renderPass
	VK_CHECK_CLEANUP(vkDestroyRenderPass(device.handle, renderPass, nullptr), renderPass, "vkDestroyRenderPass");

	// commandPool
	VK_CHECK_CLEANUP(vkDestroyCommandPool(device.handle, commandPool, nullptr), commandPool, "vkDestroyCommandPool");

	// device
	VK_CHECK_CLEANUP(vkDestroyDevice(device.handle, nullptr), device.handle, "vkDestroyDevice");

	// suface
	VK_CHECK_CLEANUP(vkDestroySurfaceKHR(instance, surface.handle, nullptr), surface.handle, "vkDestroySurfaceKHR");

	// window
	DestroyWindow(window.hWnd);
	UnregisterClass(window.name, GetModuleHandle(NULL));

	// debug
	if (debugReportCallback != VK_NULL_HANDLE)
	{
		PFN_vkDestroyDebugReportCallbackEXT FP_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		VK_CHECK_CLEANUP(FP_vkDestroyDebugReportCallbackEXT(instance, debugReportCallback, nullptr), debugReportCallback, "FP_vkDestroyDebugReportCallbackEXT");
	}

	// instance
	VK_CHECK_CLEANUP(vkDestroyInstance(instance, nullptr), instance, "vkDestroyInstance");
}





VkFormat VkU::GetDepthFormat(VkPhysicalDevice _physicalDevices, std::vector<VkFormat>* _preferedDepthFormat)
{
	if (_preferedDepthFormat != nullptr)
	{
		for (uint32_t j = 0; j != _preferedDepthFormat->size(); ++j)
		{
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(_physicalDevices, (*_preferedDepthFormat)[j], &formatProperties);

			if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				return (*_preferedDepthFormat)[j];
			}
		}
	}
	else
	{
		for (uint32_t j = 0; j != sizeof(preferedAllDepthFormats) / sizeof(VkFormat); ++j)
		{
			VkFormatProperties formatProperties;
			vkGetPhysicalDeviceFormatProperties(_physicalDevices, preferedAllDepthFormats[j], &formatProperties);

			if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				return preferedAllDepthFormats[j];
			}
		}
	}

	return VK_FORMAT_UNDEFINED;
}

VkU::Window VkU::GetWindow(uint32_t _width, uint32_t _height, const char * _title, const char * _name, WNDPROC _wndProc)
{
	Window window;
	window.hInstance = GetModuleHandle(NULL);
	window.name = _name;
	window.hWnd = NULL;

	if (_wndProc == nullptr)
		_wndProc = BasicWndProc;

	WNDCLASSEX wndClassEx;
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_HREDRAW | CS_VREDRAW;
	wndClassEx.lpfnWndProc = _wndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = GetModuleHandle(NULL);
	wndClassEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = _name;
	wndClassEx.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClassEx))
		return window;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = (long)_width;
	windowRect.bottom = (long)_height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	window.hWnd = CreateWindowEx(
		0,
		_name,
		_title,
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	if (window.hWnd == NULL)
		return window;

	uint32_t x = (screenWidth - windowRect.right) / 2;
	uint32_t y = (screenHeight - windowRect.bottom) / 2;
	SetWindowPos(window.hWnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	ShowWindow(window.hWnd, SW_SHOW);
	SetForegroundWindow(window.hWnd);
	SetFocus(window.hWnd);

	return window;
}

bool VkU::CheckQueueFamilyIndexSupport(uint32_t _familyIndex, PhysicalDevice _physicalDevice, VkSurfaceKHR _surface, VkQueueFlags _flags, VkBool32 _presentability, uint32_t _count)
{
	VkBool32 surfaceSupported = VK_FALSE;
	if (_surface != VK_NULL_HANDLE)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice.handle, _familyIndex, _surface, &surfaceSupported);
		if (surfaceSupported == VK_FALSE)
			return false;
	}

	if ((_flags & _physicalDevice.queueFamilyProperties[_familyIndex].queueFlags) != _flags)
		return false;

	if (_presentability == VK_TRUE && _physicalDevice.queueFamilyPresentable[_familyIndex] == VK_FALSE)
		return false;

	if (_count > _physicalDevice.queueFamilyProperties[_familyIndex].queueCount)
		return false;

	return true;
}
std::vector<uint32_t> VkU::GetQueueFamilyIndicesWithSupport(Queue _deviceQueue, PhysicalDevice _physicalDevice, std::vector<Surface> _surfaces)
{
	std::vector<uint32_t> indices;

	for (uint32_t i = 0; i != (uint32_t)_physicalDevice.queueFamilyProperties.size(); ++i)
	{
		bool allSurfacesValid = true;
		for (size_t s = 0; s != _surfaces.size(); ++s)
		{
			if (!CheckQueueFamilyIndexSupport(i, _physicalDevice, _surfaces[s].handle, _deviceQueue.flags, _deviceQueue.presentability, _deviceQueue.count))
			{
				allSurfacesValid = false;
				break;
			}
		}

		if (allSurfacesValid)
			indices.push_back(i);
	}

	return indices;
}
bool VkU::PickDeviceQueuesIndicesRecursively(std::vector<uint32_t>& _queueFamilyUseCount, std::vector<std::vector<uint32_t>> _deviceQueuesValidIndices, std::vector<VkQueueFamilyProperties> _queueFamilyProperties, std::vector<std::array<uint32_t, 3>>& _queueFamily_Indices_Count, size_t _depth)
{
	if (_depth == _deviceQueuesValidIndices.size())
		return true; // nothing to left to assign, therefore we succeed

	for (size_t i = 0; i != _deviceQueuesValidIndices[_depth].size(); ++i)
	{
		if (_queueFamilyUseCount[_deviceQueuesValidIndices[_depth][i]] + _queueFamily_Indices_Count[_depth][2] <= _queueFamilyProperties[_deviceQueuesValidIndices[_depth][i]].queueCount)
		{

			uint32_t queueIndex = _queueFamilyUseCount[_deviceQueuesValidIndices[_depth][i]];
			uint32_t queueFamilyIndex = _deviceQueuesValidIndices[_depth][i];
			_queueFamilyUseCount[_deviceQueuesValidIndices[_depth][i]] += _queueFamily_Indices_Count[_depth][2];

			if (PickDeviceQueuesIndicesRecursively(_queueFamilyUseCount, _deviceQueuesValidIndices, _queueFamilyProperties, _queueFamily_Indices_Count, _depth + 1))
			{
				_queueFamily_Indices_Count[_depth][0] = queueFamilyIndex;
				_queueFamily_Indices_Count[_depth][1] = queueIndex;
				return true;
			}
			else
			{
				_queueFamilyUseCount[_deviceQueuesValidIndices[_depth][i]] -= _queueFamily_Indices_Count[_depth][2];
			}
		}
	}

	return false;
}
std::vector<VkU::Queue> VkU::PickDeviceQueuesIndices(std::vector<Queue> _queues, PhysicalDevice _physicalDevice, std::vector<Surface> _surfaces, bool * _isCompatible)
{
	std::vector<std::vector<uint32_t>> deviceQueuesValidIndices;

	// get possible indices
	for (size_t q = 0; q != _queues.size(); ++q)
	{
		std::vector<uint32_t> indices = GetQueueFamilyIndicesWithSupport(_queues[q], _physicalDevice, _surfaces);

		if (indices.size() == 0)
		{
			if (_isCompatible != nullptr)
				*_isCompatible = false;

			return _queues; // one queue cannot be represented
		}
		else
			deviceQueuesValidIndices.push_back(indices);
	}

	// assign indices
	std::vector<uint32_t> queueFamilyUseCount(_physicalDevice.queueFamilyProperties.size());
	std::vector<std::array<uint32_t, 3>> queueFamily_Indices_Count(_queues.size());
	for (size_t i = 0; i != queueFamily_Indices_Count.size(); ++i)
		queueFamily_Indices_Count[i][2] = _queues[i].count;

	*_isCompatible = PickDeviceQueuesIndicesRecursively(queueFamilyUseCount, deviceQueuesValidIndices, _physicalDevice.queueFamilyProperties, queueFamily_Indices_Count, 0);

	for (size_t q = 0; q != _queues.size(); ++q)
	{
		_queues[q].queueFamilyIndex = queueFamily_Indices_Count[q][0];
		_queues[q].queueIndex = queueFamily_Indices_Count[q][1];
	}

	return _queues;
}

std::vector<VkSurfaceFormatKHR> VkU::GetVkSurfaceFormatKHRs(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface)
{
	std::vector<VkSurfaceFormatKHR> surfaceFormats;

	uint32_t propertyCount = 0;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &propertyCount, nullptr), "????????????????", "vkGetPhysicalDeviceSurfaceFormatsKHR");
	surfaceFormats.resize(propertyCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &propertyCount, surfaceFormats.data()), "????????????????", "vkGetPhysicalDeviceSurfaceFormatsKHR");

	return surfaceFormats;
}
VkSurfaceFormatKHR VkU::GetVkSurfaceFormatKHR(VkPhysicalDevice _physicalDevice, Surface _surface, std::vector<VkFormat>* _preferedColorFormats)
{
	std::vector<VkSurfaceFormatKHR> surfaceFormats = GetVkSurfaceFormatKHRs(_physicalDevice, _surface.handle);

	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		if (_preferedColorFormats != nullptr)
			return{ (*_preferedColorFormats)[0], surfaceFormats[0].colorSpace };
		else
			return{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	}
	else if (_preferedColorFormats != nullptr)
	{
		for (uint32_t i = 0; i != (*_preferedColorFormats).size(); ++i)
		{
			for (uint32_t j = 0; j != surfaceFormats.size(); ++j)
			{
				if ((*_preferedColorFormats)[i] == surfaceFormats[j].format)
				{
					return{ surfaceFormats[i] };
					break;
				}
			}
		}
	}

	return surfaceFormats[0];
}

VkCompositeAlphaFlagBitsKHR VkU::GetVkCompositeAlphaFlagBitsKHR(VkSurfaceCapabilitiesKHR _surfaceCapabilities, std::vector<VkCompositeAlphaFlagBitsKHR>* _preferedCompositeAlphas)
{
	if (_preferedCompositeAlphas != nullptr)
	{
		for (size_t i = 0; i != _preferedCompositeAlphas->size(); ++i)
		{
			if (((*_preferedCompositeAlphas)[i] & _surfaceCapabilities.supportedCompositeAlpha) == (*_preferedCompositeAlphas)[i])
			{
				return (*_preferedCompositeAlphas)[i];
			}
		}
	}

	return VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
}
VkSurfaceCapabilitiesKHR VkU::GetVkSurfaceCapabilitiesKHR(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCapabilities), "????????????????", "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

	return surfaceCapabilities;
}

std::vector<VkPresentModeKHR> VkU::GetVkPresentModeKHRs(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface)
{
	std::vector<VkPresentModeKHR> presentModes;

	uint32_t propertyCount = 0;
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &propertyCount, nullptr), "????????????????", "vkGetPhysicalDeviceSurfacePresentModesKHR");
	presentModes.resize(propertyCount);
	VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &propertyCount, presentModes.data()), "????????????????", "vkGetPhysicalDeviceSurfacePresentModesKHR");

	return presentModes;
}
VkPresentModeKHR VkU::GetVkPresentModeKHR(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface, std::vector<VkPresentModeKHR>* _preferedPresentModes)
{
	if (_preferedPresentModes != nullptr)
	{
		std::vector<VkPresentModeKHR> presentModes = VkU::GetVkPresentModeKHRs(_physicalDevice, _surface);

		for (size_t i = 0; i != _preferedPresentModes->size(); ++i)
		{
			for (size_t j = 0; j != presentModes.size(); ++j)
			{
				if (presentModes[j] == (*_preferedPresentModes)[i])
				{
					return (*_preferedPresentModes)[i];
				}
			}
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

static uint32_t VkU::FindMemoryTypeIndex(VkMemoryRequirements _memoryRequirements, PhysicalDevice _physicalDevice, VkMemoryPropertyFlags _memoryPropertyFlags)
{
	for (uint32_t i = 0; i != _physicalDevice.memoryProperties.memoryTypeCount; ++i)
	{
		if ((_memoryRequirements.memoryTypeBits & (1 << i)) && (_physicalDevice.memoryProperties.memoryTypes[i].propertyFlags & _memoryPropertyFlags) == _memoryPropertyFlags)
		{
			return i;
		}
	}

	return -1;
}

VkU::Buffer VkU::CreateUniformBuffer(VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkDeviceSize _size)
{
	VkU::Buffer buffer;

	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = _size;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;
	VK_CHECK_RESULT(vkCreateBuffer(_vkDevice, &bufferCreateInfo, nullptr, &buffer.handle), buffer.handle, "vkCreateBuffer");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_vkDevice, buffer.handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VkU::FindMemoryTypeIndex(memoryRequirements, _physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(_vkDevice, &memoryAllocateInfo, nullptr, &buffer.memory), buffer.memory, "vkAllocateMemory");

	VK_CHECK_RESULT(vkBindBufferMemory(_vkDevice, buffer.handle, buffer.memory, 0), "????????????????", "vkBindBufferMemory");

	return buffer;
}
VkU::Buffer VkU::CreateVertexBuffer(VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkDeviceSize _size)
{
	VkU::Buffer buffer;

	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = _size;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;
	VK_CHECK_RESULT(vkCreateBuffer(_vkDevice, &bufferCreateInfo, nullptr, &buffer.handle), buffer.handle, "vkCreateBuffer");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_vkDevice, buffer.handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VkU::FindMemoryTypeIndex(memoryRequirements, _physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(_vkDevice, &memoryAllocateInfo, nullptr, &buffer.memory), buffer.memory, "vkAllocateMemory");

	VK_CHECK_RESULT(vkBindBufferMemory(_vkDevice, buffer.handle, buffer.memory, 0), "????????????????", "vkBindBufferMemory");

	return buffer;
}
VkU::Buffer VkU::CreateIndexBuffer(VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkDeviceSize _size)
{
	VkU::Buffer buffer;

	VkBufferCreateInfo bufferCreateInfo;
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.size = _size;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.queueFamilyIndexCount = 0;
	bufferCreateInfo.pQueueFamilyIndices = nullptr;
	VK_CHECK_RESULT(vkCreateBuffer(_vkDevice, &bufferCreateInfo, nullptr, &buffer.handle), buffer.handle, "vkCreateBuffer");

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(_vkDevice, buffer.handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VkU::FindMemoryTypeIndex(memoryRequirements, _physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(_vkDevice, &memoryAllocateInfo, nullptr, &buffer.memory), buffer.memory, "vkAllocateMemory");

	VK_CHECK_RESULT(vkBindBufferMemory(_vkDevice, buffer.handle, buffer.memory, 0), "????????????????", "vkBindBufferMemory");

	return buffer;
}
VkU::Buffer VkU::CreateStagingBuffer(VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkDeviceSize _size)
{
	VkU::Buffer stagingBuffer;

	// create
	{
		VkBufferCreateInfo stagingBufferCreateInfo;
		stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferCreateInfo.pNext = nullptr;
		stagingBufferCreateInfo.flags = 0;
		stagingBufferCreateInfo.size = _size;
		stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		stagingBufferCreateInfo.queueFamilyIndexCount = 0;
		stagingBufferCreateInfo.pQueueFamilyIndices = nullptr;
		VK_CHECK_RESULT(vkCreateBuffer(_vkDevice, &stagingBufferCreateInfo, nullptr, &stagingBuffer.handle), stagingBuffer.handle, "vkCreateBuffer");

		VkMemoryRequirements stagingMemoryRequirements;
		vkGetBufferMemoryRequirements(_vkDevice, stagingBuffer.handle, &stagingMemoryRequirements);

		VkMemoryAllocateInfo stagingMemoryAllocateInfo;
		stagingMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		stagingMemoryAllocateInfo.pNext = nullptr;
		stagingMemoryAllocateInfo.allocationSize = stagingMemoryRequirements.size;
		stagingMemoryAllocateInfo.memoryTypeIndex = VkU::FindMemoryTypeIndex(stagingMemoryRequirements, _physicalDevice, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(_vkDevice, &stagingMemoryAllocateInfo, nullptr, &stagingBuffer.memory), stagingBuffer.memory, "vkAllocateMemory");

		VK_CHECK_RESULT(vkBindBufferMemory(_vkDevice, stagingBuffer.handle, stagingBuffer.memory, 0), 0, "vkBindBufferMemory");
	}

	return stagingBuffer;
}
void VkU::FillStagingBuffer(VkDevice _vkDevice, Buffer _stagingBuffer, VkDeviceSize _size, void* _data)
{
	// fill
	{
		void* data;
		VK_CHECK_RESULT(vkMapMemory(_vkDevice, _stagingBuffer.memory, 0, _size, 0, &data), data, "vkMapMemory");
		memcpy(data, _data, _size);
		vkUnmapMemory(_vkDevice, _stagingBuffer.memory);
	}
}
void VkU::TransferStagingBuffer(Device _device, VkCommandBuffer _commandBuffer, VkFence& _fence, Buffer _stagingBuffer, Buffer _dstBuffer, VkDeviceSize _size)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo;
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;
	VK_CHECK_RESULT(vkWaitForFences(_device.handle, 1, &_fence, VK_TRUE, -1), "????????????????", "vkWaitForFences");
	VK_CHECK_RESULT(vkResetFences(_device.handle, 1, &_fence), "????????????????", "vkResetFences");
	VK_CHECK_RESULT(vkBeginCommandBuffer(_commandBuffer, &commandBufferBeginInfo), "????????????????", "vkBeginCommandBuffer");

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = _size;

	vkCmdCopyBuffer(_commandBuffer, _stagingBuffer.handle, _dstBuffer.handle, 1, &copyRegion);

	VK_CHECK_RESULT(vkEndCommandBuffer(_commandBuffer), 0, "vkMapMemory");

	VkSubmitInfo submitInfo;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	VK_CHECK_RESULT(vkQueueSubmit(_device.queues[GRAPHICS_PRESENT_QUEUE_INDEX].handles[0], 1, &submitInfo, _fence), 0, "vkMapMemory");
}
void VkU::DestroyBuffer(VkDevice _vkDevice, Buffer _buffer)
{
	VK_CHECK_CLEANUP(vkDestroyBuffer(_vkDevice, _buffer.handle, nullptr), _buffer.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(_vkDevice, _buffer.memory, nullptr), _buffer.memory, "vkFreeMemory");
}

void VkU::CreateSampledImage(VkDevice _vkDevice, PhysicalDevice _physicalDevice, Image& _image, VkFormat _format, VkExtent3D _extent3D)
{
	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = _format;
	imageCreateInfo.extent = _extent3D;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	VK_CHECK_RESULT(vkCreateImage(_vkDevice, &imageCreateInfo, nullptr, &_image.handle), _image.handle, "vkCreateImage");
	
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_vkDevice, _image.handle, &memoryRequirements);
	
	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VkU::FindMemoryTypeIndex(memoryRequirements, _physicalDevice, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(_vkDevice, &memoryAllocateInfo, nullptr, &_image.memory), _image.memory, "vkAllocateMemory");

	VK_CHECK_RESULT(vkBindImageMemory(_vkDevice, _image.handle, _image.memory, 0), "????????????????", "vkBindImageMemory");
}
void VkU::CreateStagingImage(VkDevice _vkDevice, PhysicalDevice _physicalDevice, Image& _image, VkFormat _format, VkExtent3D _extent3D)
{
	VkImageCreateInfo imageCreateInfo;
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = 0;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = _format;
	imageCreateInfo.extent = _extent3D;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	VK_CHECK_RESULT(vkCreateImage(_vkDevice, &imageCreateInfo, nullptr, &_image.handle), _image.handle, "vkCreateImage");

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(_vkDevice, _image.handle, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo;
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = VkU::FindMemoryTypeIndex(memoryRequirements, _physicalDevice, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(_vkDevice, &memoryAllocateInfo, nullptr, &_image.memory), _image.memory, "vkAllocateMemory");

	VK_CHECK_RESULT(vkBindImageMemory(_vkDevice, _image.handle, _image.memory, 0), "????????????????", "vkBindImageMemory");
}
void VkU::FillStagingColorImage(VkDevice _vkDevice, Image& _image, uint32_t _width, uint32_t _height, VkDeviceSize _size, void* _data)
{
	VkImageSubresource imageSubresource;
	imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresource.mipLevel = 0;
	imageSubresource.arrayLayer = 0;

	VkSubresourceLayout subresourceLayout;
	vkGetImageSubresourceLayout(_vkDevice, _image.handle, &imageSubresource, &subresourceLayout);

	void* vkData;
	VK_CHECK_RESULT(vkMapMemory(_vkDevice, _image.memory, 0, _size, 0, &vkData), vkData, "vkMapMemory");

	if (subresourceLayout.rowPitch == _width * 4)
	{
		memcpy(vkData, _data, _size);
	}
	else
	{
		uint8_t* _data8b = (uint8_t*)_data;
		uint8_t* data8b = (uint8_t*)vkData;

		for (uint32_t y = 0; y < _height; y++)
		{
			memcpy(&data8b[y * subresourceLayout.rowPitch], &_data8b[y * _width * 4], _width * 4);
		}
	}

	vkUnmapMemory(_vkDevice, _image.memory);
}
void VkU::CreateColorView(VkDevice _vkDevice, Image& _image, VkFormat _format)
{
	VkImageViewCreateInfo imageViewCreateInfo;
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.flags = VK_RESERVED_FOR_FUTURE_USE;
	imageViewCreateInfo.image = _image.handle;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = _format;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;

	VK_CHECK_RESULT(vkCreateImageView(_vkDevice, &imageViewCreateInfo, nullptr, &_image.view), _image.view, "vkCreateImageView");
}
void VkU::DestroyImage(VkDevice _vkDevice, Image _image)
{
	VK_CHECK_CLEANUP(vkDestroyImageView(_vkDevice, _image.view, nullptr), _image.view, "vkDestroyImageView");
	VK_CHECK_CLEANUP(vkDestroyImage(_vkDevice, _image.handle, nullptr), _image.handle, "vkDestroyBuffer");
	VK_CHECK_CLEANUP(vkFreeMemory(_vkDevice, _image.memory, nullptr), _image.memory, "vkFreeMemory");
}

void VkU::WaitFence(VkDevice _vkDevice, uint32_t _fenceCount, VkFence * _fences, VkBool32 _waitAll, uint64_t _timeout)
{
	VK_CHECK_RESULT(vkWaitForFences(_vkDevice, _fenceCount, _fences, _waitAll, _timeout), "????????????????", "vkWaitForFences");
}
void VkU::ResetFence(VkDevice _vkDevice, uint32_t _fenceCount, VkFence * _fences)
{
	VK_CHECK_RESULT(vkResetFences(_vkDevice, _fenceCount, _fences), "????????????????", "vkResetFences");
}
void VkU::WaitResetFence(VkDevice _vkDevice, uint32_t _fenceCount, VkFence* _fences, VkBool32 _waitAll, uint64_t _timeout)
{
	VK_CHECK_RESULT(vkWaitForFences(_vkDevice, _fenceCount, _fences, _waitAll, _timeout), "????????????????", "vkWaitForFences");
	VK_CHECK_RESULT(vkResetFences(_vkDevice, _fenceCount, _fences), "????????????????", "vkResetFences");
}

void VkU::LoadShader(const char* _filename, size_t& _fileSize, char** _buffer)
{
	std::ifstream file(_filename, std::ios::ate | std::ios::binary);

	if (file.is_open())
	{
		_fileSize = (size_t)file.tellg();
		file.seekg(0);

		if (file.good())
		{
			*_buffer = new char[_fileSize];
			file.read(*_buffer, _fileSize);
		}
		else
		{
			_fileSize = 0;
		}
	}

	file.close();
}
void VkU::LoadModel(const char* _filename, Meshes& _meshes, aiPostProcessSteps _aiPostProcessSteps)
{
	Assimp::Importer Importer;
	const aiScene* pScene;

	// forces meshes to be triangulated
	if ((_aiPostProcessSteps & aiProcess_Triangulate) != aiProcess_Triangulate)
		_aiPostProcessSteps = (aiPostProcessSteps)(_aiPostProcessSteps | aiProcess_Triangulate);

	// Open File
	pScene = Importer.ReadFile(_filename, _aiPostProcessSteps);
	if (pScene == nullptr)
	{
#if _DEBUG
		logger << "ERROR: MODEL \"" << _filename << "\" missing. Time: " << Engine::timer.GetTime() << " file = " << __FILE__ << "line = " << __LINE__ << '\n';
		assert(0);
#endif
	}

	_meshes.vertexSize = 0;
	_meshes.vertexData = nullptr;

	_meshes.indexSize = 0;
	_meshes.indexData = nullptr;

	// get number of meshes
	_meshes.meshProperties.resize(pScene->mNumMeshes);

	uint32_t indexCount = 0;
	const aiMesh* currMesh;
	VERTEX_TYPE vertexType = (VERTEX_TYPE)VERTEX_ATTRIBUTES::POS3;

	// get meshes properties
	for (unsigned int i = 0; i != pScene->mNumMeshes; ++i)
	{
		currMesh = pScene->mMeshes[i];

		_meshes.meshProperties[i].type = (VERTEX_TYPE)VERTEX_ATTRIBUTES::POS3;

		// get mesh type
		if (currMesh->HasTextureCoords(0))
		{
			vertexType = (VERTEX_TYPE)(vertexType | VERTEX_ATTRIBUTES::UV);
			_meshes.meshProperties[i].type = (VERTEX_TYPE)(_meshes.meshProperties[i].type | VERTEX_ATTRIBUTES::UV);
		}
		if (currMesh->HasNormals())
		{
			vertexType = (VERTEX_TYPE)(vertexType | VERTEX_ATTRIBUTES::NORM);
			_meshes.meshProperties[i].type = (VERTEX_TYPE)(_meshes.meshProperties[i].type | VERTEX_ATTRIBUTES::NORM);
		}
		if (currMesh->HasTangentsAndBitangents())
		{
			vertexType = (VERTEX_TYPE)(vertexType | VERTEX_ATTRIBUTES::TAN_BITAN);
			_meshes.meshProperties[i].type = (VERTEX_TYPE)(_meshes.meshProperties[i].type | VERTEX_ATTRIBUTES::TAN_BITAN);
		}

		// record index offsets
		_meshes.meshProperties[i].offset = indexCount;
		// record index count
		_meshes.meshProperties[i].indexCount = currMesh->mNumFaces * 3;

		// add index count
		indexCount += currMesh->mNumFaces * 3;

		// add vertex count
		_meshes.vertexSize += currMesh->mNumVertices;
	}

	// find index size
	_meshes.indexSize = indexCount * sizeof(uint32_t);

	// find vertex size
	uint64_t singleVertexSize = 0;
	if ((vertexType & VERTEX_ATTRIBUTES::POS3) == VERTEX_ATTRIBUTES::POS3)
		singleVertexSize += sizeof(float) * 3;
	if ((vertexType & VERTEX_ATTRIBUTES::UV) == VERTEX_ATTRIBUTES::UV)
		singleVertexSize += sizeof(float) * 2;
	if ((vertexType & VERTEX_ATTRIBUTES::NORM) == VERTEX_ATTRIBUTES::NORM)
		singleVertexSize += sizeof(float) * 3;
	if ((vertexType & VERTEX_ATTRIBUTES::TAN_BITAN) == VERTEX_ATTRIBUTES::TAN_BITAN)
		singleVertexSize += sizeof(float) * 6;

	_meshes.vertexSize *= singleVertexSize;

	// allocate buffers
	_meshes.indexData = new uint8_t[_meshes.indexSize];
	ZeroMemory(_meshes.indexData, _meshes.indexSize);
	_meshes.vertexData = new uint8_t[_meshes.vertexSize];
	ZeroMemory(_meshes.vertexData, _meshes.vertexSize);

	// fill indices
	uint64_t dataPos = 0;
	for (unsigned int i = 0; i != pScene->mNumMeshes; ++i)
	{
		currMesh = pScene->mMeshes[i];

		for (unsigned int j = 0; j != currMesh->mNumFaces; ++j)
		{
			// make sure it's a triangle
			const aiFace& face = currMesh->mFaces[j];
			if (face.mNumIndices != 3)
				continue;

			face.mIndices[0] += _meshes.meshProperties[i].offset;
			face.mIndices[1] += _meshes.meshProperties[i].offset;
			face.mIndices[2] += _meshes.meshProperties[i].offset;

			memcpy(&_meshes.indexData[dataPos], face.mIndices, sizeof(uint32_t) * 3);
			dataPos += sizeof(uint32_t) * 3;
		}
	}

	// fill vertices
	dataPos = 0;
	for (unsigned int i = 0; i != pScene->mNumMeshes; ++i)
	{
		currMesh = pScene->mMeshes[i];

		for (auto j = 0; j != currMesh->mNumVertices; ++j)
		{
			// position
			memcpy(&_meshes.vertexData[dataPos], &currMesh->mVertices[j].x, sizeof(float) * 3);
			dataPos += sizeof(float) * 3;

			// uv
			if ((vertexType & VERTEX_ATTRIBUTES::UV) == VERTEX_ATTRIBUTES::UV)
			{
				float uv[2] = { currMesh->mTextureCoords[0][j].x, -currMesh->mTextureCoords[0][j].y };
				memcpy(&_meshes.vertexData[dataPos], &uv, sizeof(float) * 2);
				dataPos += sizeof(float) * 2;
			}

			// normal
			if ((vertexType & VERTEX_ATTRIBUTES::NORM) == VERTEX_ATTRIBUTES::NORM)
			{
				memcpy(&_meshes.vertexData[dataPos], &currMesh->mNormals[j].x, sizeof(float) * 3);
				dataPos += sizeof(float) * 3;
			}

			// tangent / bitangent
			if ((vertexType & VERTEX_ATTRIBUTES::TAN_BITAN) == VERTEX_ATTRIBUTES::TAN_BITAN)
			{
				memcpy(&_meshes.vertexData[dataPos], &currMesh->mTangents[j].x, sizeof(float) * 3);
				dataPos += sizeof(float) * 3;
				memcpy(&_meshes.vertexData[dataPos], &currMesh->mBitangents[j].x, sizeof(float) * 3);
				dataPos += sizeof(float) * 3;
			}
		}
	}

	int ii = 0;
}
void VkU::LoadImageTGA(const char * _filename, uint32_t & _width, uint32_t & _height, uint8_t & _channelCount, uint8_t & _bytesPerChannel, void*& _data)
{
	_width = 0;
	_height = 0;
	_channelCount = 0;
	_bytesPerChannel = 0;
	if (_data != nullptr)
		delete[] _data;
	_data = nullptr;

	uint8_t header[12];

	FILE* fTGA;
	fTGA = fopen(_filename, "rb");

	uint8_t cTGAcompare[12] = { 0,0,10,0,0,0,0,0,0,0,0,0 };
	uint8_t uTGAcompare[12] = { 0,0, 2,0,0,0,0,0,0,0,0,0 };

	if (fTGA == NULL)
	{
#if _DEBUG
		logger << "ERROR: TGA \"" << _filename << "\" missing. Time: " << Engine::timer.GetTime() << " file = " << __FILE__ << "line = " << __LINE__ << '\n';
		assert(0);
#endif
	}

	if (fread(&header, sizeof(header), 1, fTGA) == 0)
		return;
	if (memcmp(cTGAcompare, &header, sizeof(header)) == 0)
	{
		// TODO: Load Compressed
#if _DEBUG
		logger << "ERROR: TGA compressed \"" << _filename << "\" not supported. Time: " << Engine::timer.GetTime() << " file = " << __FILE__ << "line = " << __LINE__ << '\n';
		assert(0);
#endif
	}
	else if (memcmp(uTGAcompare, &header, sizeof(header)) == 0)
	{
		uint8_t header[6];

		if (fread(header, sizeof(header), 1, fTGA) == 0)
			return;

		_width = header[1] * 256 + header[0];
		_height = header[3] * 256 + header[2];
		uint8_t bpp = header[4];

		if (_width == 0 || _height == 0 || (bpp != 24 && bpp != 32))
		{
#if _DEBUG
			logger << "ERROR: TGA uncompressed \"" << _filename << "\" contains invalid data (width = " << _width << ", height = " << _height << ", bpp = " << bpp << "). Time: " << Engine::timer.GetTime() << " file = " << __FILE__ << "line = " << __LINE__ << '\n';
			assert(0);
#endif
		}

		if (bpp == 24)
		{
			_channelCount = 3;
			_bytesPerChannel = 1;
		}
		else if (bpp == 32)
		{
			_channelCount = 4;
			_bytesPerChannel = 1;
		}

		size_t size = _channelCount * _bytesPerChannel * _width * _height;

		_data = new uint8_t[size];
		if (fread(_data, 1, size, fTGA) != size)
		{
			_width = 0;
			_height = 0;
			_channelCount = 0;
			_bytesPerChannel = 0;
			delete[] _data;
			_data = nullptr;
#if _DEBUG
			logger << "ERROR: TGA \"" << _filename << "\" failed to fread. Time: " << Engine::timer.GetTime() << " file = " << __FILE__ << "line = " << __LINE__ << '\n';
			assert(0);
#endif
			return;
		}
	}
	else
	{
		_width = 0;
		_height = 0;
		_channelCount = 0;
		_bytesPerChannel = 0;
		delete[] _data;
		_data = nullptr;
#if _DEBUG
		logger << "ERROR: TGA header \"" << _filename << "\" contains invalid data. Time: " << Engine::timer.GetTime() << " file = " << __FILE__ << "line = " << __LINE__ << '\n';
		assert(0);
#endif
		return;
	}
}
