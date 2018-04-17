#ifndef	RENDERER_H
#define RENDERER_H

#include <array>
#include <vector>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.h>
#define VK_RESERVED_FOR_FUTURE_USE 0

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include "Logger.h"

static VkResult vkResult;

#if _DEBUG
static Logger logger;
static Logger debugReportCallbackLogger;
static Logger cleanUpLogger;

#define VK_CHECK_RESULT(call, variable, callName) vkResult = call;	\
logger << "Result: " << vkResult << "	Value: " << variable << "	Time: " << Engine::timer.GetTime() << "	Call: " << callName << '\n'
#define VK_CHECK_CLEANUP(call, variable, callName) call;	\
cleanUpLogger << "Value: " << variable << "	Time: " << Engine::timer.GetTime() << "	Call: " << callName << '\n'; \
variable = VK_NULL_HANDLE

#else
#define VK_CHECK_RESULT(call, variable, callName) vkResult = call
#define VK_CHECK_CLEANUP(call, variable, callName) call
#endif

namespace VkU
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT _flags, VkDebugReportObjectTypeEXT _objType, uint64_t _obj, size_t _location, int32_t _code, const char* _layerPrefix, const char* _msg, void* _userData)
	{
#if _DEBUG
		if (_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
			debugReportCallbackLogger << "	INFORMATION:";
		if (_flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			debugReportCallbackLogger << "WARNING:";
		if (_flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			debugReportCallbackLogger << "PERFORMANCE:";
		if (_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			debugReportCallbackLogger << "ERROR:";
		if (_flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			debugReportCallbackLogger << "	DEBUG:";

		debugReportCallbackLogger << _msg << '\n';
#endif
		return VK_FALSE; // Don't abort the function that made this call
	}
	static LRESULT CALLBACK BasicWndProc(HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
	{
		switch (_uMsg)
		{
		case WM_CLOSE:
			DestroyWindow(_hWnd);
			break;
		case WM_PAINT:
			ValidateRect(_hWnd, NULL);
			break;
		}

		return (DefWindowProc(_hWnd, _uMsg, _wParam, _lParam));
	}

	static VkFormat preferedAllDepthFormats[] = {
		VK_FORMAT_D32_SFLOAT, // 100% availability 3/d1/2017
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_X8_D24_UNORM_PACK32,
		VK_FORMAT_D16_UNORM, // 100% availability 3/d1/2017
		VK_FORMAT_D16_UNORM_S8_UINT,
	};
	static std::vector<VkCompositeAlphaFlagBitsKHR> preferedCompositeAlphas = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};
	static std::vector<VkPresentModeKHR> preferedPresentModes = {
		VK_PRESENT_MODE_MAILBOX_KHR,
	};

	struct PhysicalDevice
	{
		VkPhysicalDevice						handle;

		VkPhysicalDeviceFeatures				features;
		VkPhysicalDeviceMemoryProperties		memoryProperties;
		VkPhysicalDeviceProperties				properties;
		std::vector<VkQueueFamilyProperties>	queueFamilyProperties;
		std::vector<VkBool32>					queueFamilyPresentable;

		VkFormat depthFormat;
	};
	struct Window
	{
		HWND		hWnd;
		HINSTANCE	hInstance;
		const char*	name;
	};
	struct Surface
	{
		VkSurfaceKHR				handle;
		VkSurfaceFormatKHR			colorFormat;
		VkCompositeAlphaFlagBitsKHR	compositeAlpha;
		VkPresentModeKHR			presentMode;
	};
	struct Queue
	{
		std::vector<VkQueue>	handles;

		uint32_t				queueFamilyIndex;
		uint32_t				queueIndex;

		VkQueueFlags			flags;
		VkBool32				presentability;
		float					priority;
		uint32_t				count;

		static inline Queue GetQueue(VkBool32 _presentability, VkQueueFlags _flags, float _priority, uint32_t _count)
		{
			return{ {}, (uint32_t)-1, (uint32_t)-1, _flags, _presentability, _priority, _count };
		}
	};
	struct Device
	{
		VkDevice handle;
		uint32_t physicalDeviceIndex;
		std::vector<Queue> queues;
	};
	struct Image
	{
		VkImage handle;
		VkDeviceMemory memory;
		VkImageView view;
	};
	struct Swapchain
	{
		VkSwapchainKHR handle;

		VkExtent2D extent;

		std::vector<VkImage>		images;
		std::vector<VkImageView>	views;
		std::vector<VkFramebuffer>	framebuffers;
	};

	struct Buffer
	{
		VkBuffer handle;
		VkDeviceMemory memory;
	};
	struct ShaderModule
	{
		VkShaderModule			handle;
		VkShaderStageFlagBits	stage;
		const char*				entryPointName;
	};

	enum VERTEX_ATTRIBUTES
	{
		POS2 = 1,
		POS3 = 2,
		COL = 4,
		UV = 8,
		NORM = 16,
		TAN_BITAN = 32,
	};
	enum VERTEX_TYPE
	{
		VK_UNKNOWN =				0,
		VK_POS3 =					POS3,
		VK_POS3_COL =				POS3	|	COL,
		VT_POS3_UV =				POS3	|	UV,
		VT_POS3_NORM =				POS3	|	NORM,
		VK_POS3_UV_NORM =			POS3	|	UV		|	NORM,
		VK_POS3_NORM_TAN_BITAN =	POS3	|	NORM	|	TAN_BITAN,
		VK_POS3_UV_NORM_TAN_BITAN =	POS3	|	UV		|	NORM		|		TAN_BITAN,
	};

	struct VertexPosUv
	{
		glm::vec3 position;
		glm::vec2 uv;
	};
	struct VertexPosUvNormTanBitan
	{
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	struct PointLight
	{
		glm::vec3 position;
		float padding;
		glm::vec3 color;
		float strenght;
	};

	struct Material
	{
		glm::vec3 color;

		float specularStrength;
		float specularRoughness;
	};

	struct Meshes
	{
		struct MeshProperties
		{
			VERTEX_TYPE type;
			uint32_t offset;
			uint32_t indexCount;
		};

		std::vector<MeshProperties> meshProperties;

		uint64_t vertexSize;
		uint8_t* vertexData;

		uint64_t indexSize;
		uint8_t* indexData;
	};

	VkFormat GetDepthFormat(VkPhysicalDevice _physicalDevices, std::vector<VkFormat>* _preferedDepthFormat);

	Window GetWindow(uint32_t _width, uint32_t _height, const char * _title, const char * _name, WNDPROC _wndProc);

	bool CheckQueueFamilyIndexSupport(uint32_t _familyIndex, PhysicalDevice _physicalDevice, VkSurfaceKHR _surface, VkQueueFlags _flags, VkBool32 _presentability, uint32_t _count);
	std::vector<uint32_t> GetQueueFamilyIndicesWithSupport(Queue _deviceQueue, PhysicalDevice _physicalDevice, std::vector<Surface> _surfaces);
	bool PickDeviceQueuesIndicesRecursively(std::vector<uint32_t>& _queueFamilyUseCount, std::vector<std::vector<uint32_t>> _deviceQueuesValidIndices, std::vector<VkQueueFamilyProperties> _queueFamilyProperties, std::vector<std::array<uint32_t, 3>>& _queueFamily_Indices_Count, size_t _depth);
	std::vector<Queue> PickDeviceQueuesIndices(std::vector<Queue> _queues, PhysicalDevice _physicalDevice, std::vector<Surface> _surfaces, bool * _isCompatible);

	std::vector<VkSurfaceFormatKHR> GetVkSurfaceFormatKHRs(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface);
	VkSurfaceFormatKHR GetVkSurfaceFormatKHR(VkPhysicalDevice _physicalDevice, Surface _surface, std::vector<VkFormat>* _preferedColorFormats);

	VkCompositeAlphaFlagBitsKHR GetVkCompositeAlphaFlagBitsKHR(VkSurfaceCapabilitiesKHR _surfaceCapabilities, std::vector<VkCompositeAlphaFlagBitsKHR>* _preferedCompositeAlphas);
	VkSurfaceCapabilitiesKHR GetVkSurfaceCapabilitiesKHR(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface);

	std::vector<VkPresentModeKHR> GetVkPresentModeKHRs(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface);
	VkPresentModeKHR GetVkPresentModeKHR(VkPhysicalDevice _physicalDevice, VkSurfaceKHR _surface, std::vector<VkPresentModeKHR>* _preferedPresentModes);

	static uint32_t FindMemoryTypeIndex(VkMemoryRequirements _memoryRequirements, PhysicalDevice _physicalDevice, VkMemoryPropertyFlags _memoryPropertyFlags);

	static Buffer CreateUniformBuffer(VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkDeviceSize _size);
	static Buffer CreateVertexBuffer(VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkDeviceSize _size);
	static Buffer CreateIndexBuffer(VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkDeviceSize _size);
	static Buffer CreateStagingBuffer(VkDevice _vkDevice, PhysicalDevice _physicalDevice, VkDeviceSize _size);
	static void FillStagingBuffer(VkDevice _vkDevice, Buffer _stagingBuffer, VkDeviceSize _size, void* _data);
	static void TransferStagingBuffer(Device _device, VkCommandBuffer _commandBuffer, VkFence& _fence, Buffer _stagingBuffer, Buffer _dstBuffer, VkDeviceSize _size);
	static void DestroyBuffer(VkDevice _vkDevice, Buffer _buffer);

	static void CreateSampledImage(VkDevice _vkDevice, PhysicalDevice _physicalDevice, Image& _image, VkFormat _format, VkExtent3D _extent3D);
	static void CreateStagingImage(VkDevice _vkDevice, PhysicalDevice _physicalDevice, Image& _image, VkFormat _format, VkExtent3D _extent3D);
	static void FillStagingColorImage(VkDevice _vkDevice, Image& _image, uint32_t _width, uint32_t _height, VkDeviceSize _size, void* _data);
	static void CreateColorView(VkDevice _vkDevice, Image& _image, VkFormat _format);
	static void DestroyImage(VkDevice _vkDevice, Image _image);

	static void WaitFence (VkDevice _vkDevice, uint32_t _fenceCount, VkFence* _fences, VkBool32 _waitAll, uint64_t _timeout);
	static void ResetFence (VkDevice _vkDevice, uint32_t _fenceCount, VkFence* _fences);
	static void WaitResetFence(VkDevice _vkDevice, uint32_t _fenceCount, VkFence* _fences, VkBool32 _waitAll, uint64_t _timeout);

	static void LoadShader(const char* _filename, size_t& _fileSize, char** _buffer);
	static void LoadModel(const char* _filename, Meshes& _meshes, aiPostProcessSteps _aiPostProcessSteps);
	static void LoadImageTGA(const char* _filename, uint32_t& _width, uint32_t& _height, uint8_t& _channelCount, uint8_t& _bitsPerChannel, void*& _data);

}

class Renderer
{
	// init
	VkInstance instance;
	VkDebugReportCallbackEXT debugReportCallback = VK_NULL_HANDLE;
	std::vector<VkU::PhysicalDevice> physicalDevices;
	VkU::Window window;
	VkU::Surface surface;
	VkU::Device device;
	VkCommandPool commandPool;
	VkCommandBuffer setupCommandBuffer;
	VkFence setupFence;
	VkRenderPass renderPass;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	VkSampler sampler;
	VkU::Image depthImage;
	VkU::Swapchain swapchain;
	VkSemaphore semaphoreImageAvailable;
	VkSemaphore semaphoreRenderDone;

	std::vector<VkCommandBuffer> renderCommandBuffers;
	std::vector<VkFence> renderFences;

	// load
	glm::mat4 viewProjection[2];
	VkU::Buffer viewProjectionBuffer;
	VkU::Buffer viewProjectionStagingBuffer;

	uint32_t maxGpuModelMatrixCount;
	uint32_t maxGpuPointLightCount;

	std::vector<glm::mat4> modelMatrices;
	VkU::Buffer modelMatricesBuffer;
	VkU::Buffer modelMatricesStagingBuffer;

	std::vector<VkU::PointLight> pointLights;
	VkU::Buffer pointLightsBuffer;
	VkU::Buffer pointLightsStagingBuffer;

	VkU::Buffer vertexBuffer;
	VkU::Buffer indexBuffer;
	std::vector<VkU::Image> imageBuffers;

	std::vector<VkU::ShaderModule>	shaderModules;

	// setup
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;

	std::vector<std::vector<VkPipelineShaderStageCreateInfo>>	pipelineShaderStagesCreateInfos;
	std::vector<VkVertexInputBindingDescription>					vertexInputBindingDescriptions;
	std::vector<std::vector<VkVertexInputAttributeDescription>>		vertexInputAttributeDescriptions;
	std::vector<VkPipelineVertexInputStateCreateInfo>			pipelineVertexInputStateCreateInfos;
	std::vector<VkPipelineInputAssemblyStateCreateInfo>			pipelineInputAssemblyStateCreateInfos;
	std::vector<VkPipelineTessellationStateCreateInfo>			pipelineTessellationStateCreateInfos;
	std::vector<VkViewport>											viewports;
	std::vector<VkRect2D>											scissors;
	std::vector<VkPipelineViewportStateCreateInfo>				pipelineViewportStateCreateInfos;
	std::vector<VkPipelineRasterizationStateCreateInfo>			pipelineRasterizationStateCreateInfos;
	std::vector<VkPipelineMultisampleStateCreateInfo>			pipelineMultisampleStateCreateInfos;
	std::vector<VkPipelineDepthStencilStateCreateInfo>			pipelineDepthStencilStateCreateInfos;
	std::vector<VkPipelineColorBlendAttachmentState>				pipelineColorBlendAttachmentState;
	std::vector<VkPipelineColorBlendStateCreateInfo>			pipelineColorBlendStateCreateInfos;
	std::vector<VkPipelineDynamicStateCreateInfo>				pipelineDynamicStateCreateInfo;
	std::vector<VkGraphicsPipelineCreateInfo>				graphicsPipelineCreateInfos;
	std::vector<VkPipeline> pipelines;

	// render
	uint32_t swapchainImageIndex;

	double lastTime = 0.0f;
	uint64_t frameCount = 0;
	uint64_t sumFPS = 0;

public:
	glm::mat4* GetView()
	{
		return &viewProjection[0];
	}
	glm::mat4* GetProjection()
	{
		return &viewProjection[1];
	}

	void Init();

	struct ShaderProperties
	{
		const char* filename;
		VkShaderStageFlagBits stage;
		const char* entryPointName;

		static ShaderProperties GetShaderProperties(const char* _filename, VkShaderStageFlagBits _stage, const char* _entryPointName)
		{
			ShaderProperties shaderProperties;

			shaderProperties.filename = _filename;
			shaderProperties.stage = _stage;
			shaderProperties.entryPointName = _entryPointName;

			return shaderProperties;
		}
	};
	void Load(std::vector<ShaderProperties> _shaderModulesProperties, std::vector<const char*> _modelNames, std::vector<const char*> _imageNames);
	void Setup();
	void Render();
	void ShutDown();
};

#endif